from fprime_gds.common.communication.framing import FramerDeframer
from fprime_gds.plugin.definitions import gds_plugin_implementation


class AX25KissFramer(FramerDeframer):

    KISS_FEND  = 0xC0
    KISS_FESC  = 0xDB
    KISS_TFEND = 0xDC
    KISS_TFDD  = 0xDD

    # Known F' packet type bytes (FwPacketType enum values).
    # Used to validate the result of the hex-decode fallback path.
    _FP_TYPES = frozenset([1, 2, 3, 4, 5])  # COMMAND, TELEM, LOG, FILE, HAND

    def __init__(self, ax25_dest="SATSIM", ax25_src="W1AW"):
        self.dest_call = ax25_dest
        self.src_call  = ax25_src

    def frame(self, data: bytes) -> bytes:
        ax25 = (
            self._encode_callsign(self.dest_call, last=False)
            + self._encode_callsign(self.src_call,  last=True)
            + bytes([0x03, 0xF0])  # UI frame, No Layer 3 PID
            + data
        )
        return self._kiss_wrap(ax25)

    def deframe(self, data: bytes, no_copy=False):
        data = data if no_copy else bytes(data)
        while len(data) >= 2:
            if data[0] != self.KISS_FEND:
                data = data[1:]
                continue
            end = data.find(bytes([self.KISS_FEND]), 1)
            if end == -1:
                break
            raw_frame = self._kiss_unescape(data[1:end])
            data = data[end + 1:]
            if len(raw_frame) < 1 or (raw_frame[0] & 0x0F) != 0x00:
                continue
            ax25 = raw_frame[1:]  # strip KISS command byte

            # Find end of AX.25 address fields by scanning for the end-of-address
            # bit (LSB of each 7th/14th/... SSID byte). Standard frames have two
            # address fields (dest + src = 14 bytes); digipeater hops add more.
            info_start = self._ax25_info_offset(ax25)
            if info_start is None:
                continue
            payload = ax25[info_start:]
            if not payload:
                continue

            # Fallback hex-decode for the gen_packets/rpitx transmission path
            # where RadioBridge serialised binary F' bytes as ASCII hex into the
            # AX.25 info field. Only accept the decoded result when it starts with
            # a recognised F' packet type byte to avoid corrupting binary frames.
            # TODO: remove this block once the KISS-socket RadioBridge is deployed.
            try:
                decoded = bytes.fromhex(payload.decode("ascii").strip())
                if decoded and decoded[0] in self._FP_TYPES:
                    payload = decoded
            except (ValueError, UnicodeDecodeError):
                pass  # binary payload (normal path) — use as-is

            return payload, data, b""
        return None, data, b""

    def _ax25_info_offset(self, ax25: bytes) -> int | None:
        """Return byte index of the AX.25 info field, or None if frame is too short."""
        i = 0
        while i + 7 <= len(ax25):
            ssid_byte = ax25[i + 6]
            i += 7
            if ssid_byte & 0x01:  # end-of-address bit set
                # Skip control byte + PID byte (UI frames always have PID)
                i += 2
                return i if i <= len(ax25) else None
        return None  # end-of-address bit never found — malformed frame

    def _encode_callsign(self, call: str, last: bool) -> bytes:
        call = call.upper().ljust(6)[:6]
        enc = bytes([(ord(c) << 1) & 0xFE for c in call])
        ssid = 0x60 | (0x01 if last else 0x00)  # H-bit set on last address field
        return enc + bytes([ssid])

    def _kiss_wrap(self, frame: bytes) -> bytes:
        body = bytes([0x00])
        for b in frame:
            if b == self.KISS_FEND:
                body += bytes([self.KISS_FESC, self.KISS_TFEND])
            elif b == self.KISS_FESC:
                body += bytes([self.KISS_FESC, self.KISS_TFDD])
            else:
                body += bytes([b])
        return bytes([self.KISS_FEND]) + body + bytes([self.KISS_FEND])

    def _kiss_unescape(self, data: bytes) -> bytes:
        out, i = [], 0
        while i < len(data):
            if data[i] == self.KISS_FESC and i + 1 < len(data):
                nxt = data[i + 1]
                out.append(self.KISS_FEND if nxt == self.KISS_TFEND else self.KISS_FESC)
                i += 2
            else:
                out.append(data[i])
                i += 1
        return bytes(out)

    @classmethod
    def get_name(cls):
        return "ax25-kiss"

    @classmethod
    def get_arguments(cls):
        return {
            ("--ax25-dest",): {
                "type": str,
                "default": "AMSAT0",
                "help": "AX.25 destination callsign (default: AMSAT0)",
            },
            ("--ax25-src",): {
                "type": str,
                "default": "W1AW",
                "help": "AX.25 source callsign (default: W1AW)",
            },
        }

    @classmethod
    def check_arguments(cls, **kwargs):
        pass

    @classmethod
    @gds_plugin_implementation
    def register_framing_plugin(cls):
        return cls

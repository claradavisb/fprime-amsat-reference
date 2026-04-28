from fprime_gds.common.communication.framing import FramerDeframer
from fprime_gds.plugin.definitions import gds_plugin_implementation


class AX25KissFramer(FramerDeframer):

    KISS_FEND  = 0xC0
    KISS_FESC  = 0xDB
    KISS_TFEND = 0xDC
    KISS_TFDD  = 0xDD
    AX25_HEADER_SIZE = 16

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
            if len(ax25) <= self.AX25_HEADER_SIZE:
                continue
            payload = ax25[self.AX25_HEADER_SIZE:]
            return payload, data, b""
        return None, data, b""


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

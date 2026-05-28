#!/bin/bash
# PulseAudio null sinks to route audio between GNU Radio and Direwolf.
# direwolf_tx: Direwolf TX encodes to this; new_aprs.py (HackRF) reads via .monitor
# direwolf_rx: rx_aprs.py (RTL-SDR) writes here; Direwolf RX reads via .monitor
pactl load-module module-null-sink sink_name=direwolf_tx \
    sink_properties=device.description=direwolf_tx
pactl set-default-source direwolf_tx.monitor

pactl load-module module-null-sink sink_name=direwolf_rx \
    sink_properties=device.description=direwolf_rx
pactl set-default-sink direwolf_rx

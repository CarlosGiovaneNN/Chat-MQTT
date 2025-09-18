#!/bin/bash

# Mensagens publicadas
gnome-terminal -- bash -c "mosquitto_sub -h localhost -p 1883 -t '#' -v -u user1 -P password1; exec bash"

# Logs do broker
gnome-terminal -- bash -c "sudo journalctl -u mosquitto -f; exec bash"

# Tráfego bruto
gnome-terminal -- bash -c "sudo tcpdump -i any -nn -s0 -A port 1883; exec bash"

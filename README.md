# WiFi Relay/Thermostat Board

This repo contains a new version of the firmware for the single-channel
Wifi relay sold via the OpenEnergyMonitor shop [1].

Compared to the version provided by OpenEnergyMonitor, 
this version is based on Arduino ESP8266 core (2.4.2)  with these features:
- [WebSocket] support
- [MQTT] support
- [NTP] support
- [SPIFF] support
- [RF69] support (required to sold a RFM69W)
- [OTA] support (no yet tested)
 
I plan to implement on my free time new features (e.g. integrate machine learning to optimize heating, 
the management of mutli-zone).

NOTE: this firmware is not yet usable.

[1] https://guide.openenergymonitor.org/integrations/mqtt-relay/

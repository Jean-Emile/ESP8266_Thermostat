# WiFi Relay/Thermostat Board


NOTE: this firmware is not yet usable.

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




### Compile the Firmware

####Requirements: 

First, you have to install and configure Arduino ESP8266 Core by following:
https://github.com/esp8266/Arduino/blob/2.4.2/doc/installing.rst

```console
git clone https://github.com/Jean-Emile/ESP8266_Thermostat.git
cd ESP8266_Thermostat
make
make flash
make flash_fs
```



# References

[1] https://guide.openenergymonitor.org/integrations/mqtt-relay/

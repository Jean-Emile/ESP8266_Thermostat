# WiFi Relay/Thermostat Board

[![Build Status](https://travis-ci.org/Jean-Emile/ESP8266_Thermostat.svg?branch=dev)](https://travis-ci.org/Jean-Emile/ESP8266_Thermostat)

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

## Installation

#### 1. Install PlatformIO 
See [PlatformIO installation docs](http://docs.platformio.org/en/latest/installation.html#installer-script).

#### 2. Clone this repo

`$ git clone https://github.com/Jean-Emile/ESP8266_Thermostat`

#### 3. Compile

```
$ cd ESP8266_Thermostat
$ pio run
```

#### 3. Upload

- Put ESP8266 into bootloader mode by pressing the reset button then Upload main program:

`$ pio run -t upload`

-  Put ESP8266 into bootloader mode again and then

`$ pio run -t uploadfs`

## Configuration
The thermostat configuration is available in the folder "src/data/config.json"

#### Config.json:
```json
{
  "wifi": {
    "mode": 0,
    "ssid":"",
    "password":""
  },
  "mqtt":  {
    "server":"",
    "user":"",
    "password": "",
    "port":1883
  },
  "ntp":{
    "tz":7200
  },
  "smtp":{
    "server": "smtp.gmail.com",
    "login":"",
    "password": "",
    "from": "",
    "port":465
  }
}
```


####  Wifi

In the config.json file you have to configure your ssid and password.

####  MQTT


#### STMP Server (Notifier)

Using SMTP Authentication we provide only email and password,

by default Google uses more complex verification methods so we need to change settings.

Go to your Google account settings (https://myaccount.google.com/security#activity) and enable "Allow less secure apps" at the bottom of the page.

base64encode.org for encoding your login and password  

# References
[1] https://guide.openenergymonitor.org/integrations/mqtt-relay/


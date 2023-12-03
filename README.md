# EQ-3 Radiator valve control application for ESP-32

![tested on ESP-WROOM-32](https://img.shields.io/badge/tested--on-ESP--WROOM--32-brightgreen.svg)

----

**This repository is a fork of <https://github.com/softypit/esp32_mqtt_eq3>. My target is to make this to be integrated easily on Home Assistant. My repect to @softypit for making such good gateway for EQ-3 valves.**

Main changes from parent are:

- MQTT topics have been reworked for easier integration
- MQTT discovery messages are sent automatically on start to let register the valve into Home Assistant automatically.

----

EQ-3 radiator valves work really well for a home-automation heating system. They are fully configurable vie BLE as well as their front-panel. There are more features on the valves than the calorBT app makes available.

The main problem with centrally controlling EQ-3 valves is the limited range of BLE. This makes it impossible to use a single central-controller to talk to all TRVs in a typical house. Therefore multiple 'hubs' are required at distributed locations.

## Table of Contents

- [EQ-3 Radiator valve control application for ESP-32](#eq-3-radiator-valve-control-application-for-esp-32)
  - [Table of Contents](#table-of-contents)
  - [Description](#description)
    - [Configuration](#configuration)
      - [Reset configuration](#reset-configuration)
    - [Determine EQ-3 addresses](#determine-eq-3-addresses)
    - [Supported commands](#supported-commands)
    - [JSON-Format of status topic](#json-format-of-status-topic)
    - [Read current status](#read-current-status)
    - [MQTT Topics](#mqtt-topics)
    - [Web interface](#web-interface)
  - [Usage Summary](#usage-summary)
  - [Home Assistant automatic integration](#home-assistant-automatic-integration)
  - [Home Assistant example card](#home-assistant-example-card)
  - [Developer notes](#developer-notes)
  - [Testing](#testing)
  - [Supported Models](#supported-models)
  - [Credits](#credits)
    - [Source and continuative reverse engineering by](#source-and-continuative-reverse-engineering-by)
    - [Notes](#notes)
    - [Compiling](#compiling)

## Description

This application acts as a hub and uses BLE to communicate with EQ-3 TRVs and makes configuration possible via MQTT over WiFi.

When using calorBT some very basic security is employed. This security however lives in the calorBT application and not in the valve. The EQ-3 valves do not require any authentication from the BLE client to obey commands.

### Configuration

To quickly flash the application to a ESP32, download the latest release from <https://github.com/gmag11/esp32_mqtt_eq3/releases> and flash it via esptool, using

```bash
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 115200 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0xd000 ota_data_initial.bin 0x1000 bootloader.bin 0x10000 esp32_mqtt_eq3.bin 0x8000 partition-table.bin
```

in a Linux terminal with [esptool](https://github.com/espressif/esptool) installed.

On first use the ESP32 will start in access-point mode appearing as 'HeatingController'. Connect to this AP (password is 'password' or unset) and browse to the device IP address (192.168.4.1). The device configuration parameters can be set here:  

| Parameter | Description | Examples |
| ------------- | ------------- | ------------- |
| ssid | the AP to connect to for normal operation | |
| password | the password for the AP | |
| mqtturl | url to access the mqtt broker<br>allowed values:<br><br>- IP-Address, Hostname or Domainname only ("mqtt://" is used in this case)<br>- URL with scheme mqtt, ws, wss, tco, ssl (only mqtt is tested yet)| 192.168.0.2<br>mqtt://192.168.0.2<br>ws://192.168.0.2 |
| mqttuser | mqtt broker username | |
| mqttpass | mqtt broker password | |
| mqttid | the unique id for this device to use with the mqtt broker **(max 20 characters)** | livingroom |
| ntp enabled | enable network time protocol support | |
| ntp server1 | url for ntp server | pool.ntp.org |
| ntp server2 | url for ntp server | europe.pool.ntp.org |
| timezone | timezone in TZ format | GMT0BST,M3.5.0/2,M11.5.0/2 |
| ip | fixed IP for WiFi network (leave blank to DHCP) | |
| gw | gateway IP for WiFi network (leave blank for DHCP) | |
| netmask | netmask for WiFi network (leave blank for DHCP) | |
| DNS server1 | url for ntp server | 8.8.8.8 |
| DNS server2 | url for ntp server | 4.4.4.4 |

Once the ESP32 is running in client mode the configuration page can be accessed on the webserver at /config

#### Reset configuration

The application can be forced into config mode by pressing and holding the `BOOT` key AFTER the `EN` key has been released.

### Determine EQ-3 addresses

Once connected in WiFi STA mode this application first scans for EQ-3 valves and publishes their addresses and rssi to the MQTT broker.  
A scan can be initiated at any time by publishing to the `<mqttid>radin/scan` topic.  
Scan results are published to `<mqttid>radout/devlist` in json format.

```json
{
    "devices":[
        {"rssi":-77,"bleaddr":"00:1A:22:11:E7:20"},
        {"rssi":-87,"bleaddr":"00:1A:22:10:61:F3"},
        {"rssi":-81,"bleaddr":"00:1A:22:10:41:90"}
    ]
}
```

Control of valves is carried out by publishing to the `<mqttid>radin/trv/<address>/<command>` topic with a payload consisting of:
  `[parm]`
where the device is indicated by its bluetooth address (MAC)

### Supported commands

| Parameter | Description | Parameters | Examples | Stable since |
| ------------- | ------------- | ------------- | ------------- | ------------- |
| settime | sets the current time on the valve | settime has an optional parameter of the hexadecimal encoded current time.<br>parm is 12 characters hexadecimal yymmddhhMMss (e.g. 13010c0c0a00 is 2019/Jan/12 12:00.00)<br>if no parameter is submitted and ntp is enabled the ntp time (with timezone offset) will be used | *`<mqttid>radin/trv/<eq-3-address>/settemp 13010c0c0a00`*<br><br>`livingroomradin/trv/ab:cd:ef:gh:ij:kl/settemp 13010c0c0a00` | v1.20 |
| boost | sets the boost mode | -none - | *`<mqttid>radin/trv/<eq-3-address>/boost`*<br><br>`livingroomradin/trv/ab:cd:ef:gh:ij:kl/boost` | v1.20 |
| unboost | reset to unboost mode | -none - | *`<mqttid>radin/trv/<eq-3-address>/unboost`*<br><br>`livingroomradin/trv/ab:cd:ef:gh:ij:kl/unboost` | v1.20 |
| lock | locks the front-panel controls | -none - | *`<mqttid>radin/trv/<eq-3-address>/lock`*<br><br>`livingroomradin/trv/ab:cd:ef:gh:ij:kl/lock` | v1.20 |
| unlock | release the lock for the front-panel controls | -none - | *`<mqttid>radin/trv/<eq-3-address>/unlock`*<br><br>`livingroomradin/trv/ab:cd:ef:gh:ij:kl/unlock` | v1.20 |
| auto | enables the internal temperature/time program | -none - | *`<mqttid>radin/trv/<eq-3-address>/auto`*<br><br>`livingroomradin/trv/ab:cd:ef:gh:ij:kl/auto` | v1.20 |
| manual | disables the internal temperature/time program | -none - | *`<mqttid>radin/trv/<eq-3-address>/manual`*<br><br>`livingroomradin/trv/ab:cd:ef:gh:ij:kl/manual` | v1.20 |
| offset | sets the room-temperature offset | the temperature to set, this can be -3.5 - +3.5 in 0.5 degree increments | *`<mqttid>radin/trv/<eq-3-address>/offset 3.5`*<br><br>`livingroomradin/trv/ab:cd:ef:gh:ij:kl/offset 3.5` | v1.20 |
| settemp | sets the required temperature for the valve to open/close at | the temperature to set, this can be 5.0 to 29.5 in 0.5 degree increments| *`<mqttid>radin/trv/<eq-3-address>/settemp 20.0`*<br><br>`livingroomradin/trv/ab:cd:ef:gh:ij:kl/settemp 20.0` | v1.20 |
| on | opens the valve fully (lcd display 'on') | -none - | *`<mqttid>radin/trv/<eq3-address>/on`*<br><br>`livingroomradin/trv/ab:cd:ef:gh:ij:kl/on` | v1.49 |
| off | closes the valve fully (lcd display 'off') | -none - | *`<mqttid>radin/trv/<eq3-address>/off`*<br><br>`livingroomradin/trv/ab:cd:ef:gh:ij:kl/off` | v1.49 |

In response to every successful command a status message is published to `<mqttid>radout/status/<address>` containing json-encoded details of address, temperature set point, valve open percentage, mode, boost state, lock state and battery state.

This can be used as an acknowledgement of a successful command to remote mqtt clients.

### JSON-Format of status topic

| Key | Description | Exampls | Since Version |
| ------------- |  ------------- |  ------------- |  ------------- |
| trv | Bluetooth-Address of the corresponding thermostat | `"trv":"ab:cd:ef:gh:ij:kl"` | 1.20 |
| temp | the current target room-temperature is set | `"temp":"20.0"` | 1.20 |
| offsetTemp | the current offset temperature is set | `"offsetTemp":"0.0"` | 1.30 (upstream merge in dev) |
| valve | Position of valve control | `"valve": "0"` | 1.64 |
| mode | the current thermostate programm mode<br><br>`"auto"` = internal temperature/time program is used<br>`"manual"` = internal temperature/time program is disabled<br>`"holiday"` = holiday mode is used | `"mode":"auto"`<br> `"mode":"manual"`<br> `"mode":"holiday"` | 1.20 <sup>1)</sup> |
| mode_ha | Mode reported to HomeAssistant<br><br>`"off"` = temperature &lt; 5<br>`"heat"` = manual mode with temperature &gt; 5<br>`"auto"`= auto mode | `"mode": "off"`<br> `"mode": "heat"`<br> `"mode": "auto"` | x.xx |
| boost | boost-mode is active / inactiv | `"boost"`:`"active"`<br>`"boost"`:`"inactive"` | 1.20 |
| state | front-panel controls are locked / unlocked | `"state"`:`"locked"`<br>`"state"`:`"unlocked"` | 1.20 |
| battery | battery state | `"battery"`:`"GOOD"`<br>`"battery"`:`"LOW"` | 1.20 |
| window | window-mode is active / inactive | `"window"`:`"open"`<br>`"window"`:`"closed"` | |

Example:

Topic

```raw
eq3_radout/status/00:1A:22:11:E7:20
```

Data

```json
{
    "trv":"00:1A:22:11:E7:20",
    "temp":"22.0",
    "offsetTemp":"3.0",
    "valve":"64",
    "mode":"auto",
    "mode_ha":"auto",
    "boost":"inactive",
    "window":"closed",
    "state":"unlocked",
    "battery":"GOOD"
}
```

### Read current status

There is no specific command to poll the status of the valve but using any of the commands to re-set the current value will achieve the required result.

Note 1: sending settime command does not affect settings in valve and causes a status message

Note 2: It has been observed that using unboost to poll a valve can result in the valve opening as if in boost mode but without reporting boost mode active on the display or status.
It is probably not advisable to poll the valve with the unboost command.

### MQTT Topics

| Key | Description | published | subscriped |
| ------------- |  ------------- |  :-------------: |  :-------------: |
| `<mqttid>radout/devlist` | list of available bluetooth devices | X | |
| `<mqttid>radout/status/<address>` | show a status message each time a trv is contacted | X | |
| `<mqttid>radin/trv/<address>/<command> [param]` | sends a command to the trv | | X |
| `<mqttid>radin/scan` | scan for available bluetooth devices | | X |

### Web interface

When running in client mode the ESP32 presents a web interface that can be used to control TRVs and administer the EQ3-mqtt application.
Software OTA feature can be used to apply new software binary files available in future without the need for usb/serial connection.

## Usage Summary

On first boot this application uses Kolbans bootwifi code to create the wifi AP.  
Once configuration is complete and on subsequent boots the configured details are used for connection. If connection fails the application reverts to AP mode where the web interface is used to reconfigure.

## Home Assistant automatic integration

After reboot firmware scans for Equva devices. After finding them and connecting to MQTT broker it sends autodiscovery MQTT messages that Home Assistant uses to add every found device and its entities.

After running the firmware for first time you will see something like this in Home Assistant device list.

![Home assistant device after detection](HAdevice.png)

**Note**: You do not need to configure entities manually. All device registration is done automatically.

## Home Assistant example card

This is the card that I've configured for EQ-3 valve control with this firmware

![Home assistant example card](HAcard.png)

Needed code to define is like this:

```yaml
type: vertical-stack
cards:
  - type: custom:better-thermostat-ui-card
    entity: climate.eq3_xxxxxx_thermostat
    disable_window: true
    disable_summer: true
    disable_eco: true
  - type: custom:bar-card
    entities:
      - entity: sensor.eq3_xxxxxx_valve
    entity_row: true
  - type: entity
    entity: binary_sensor.eq3_xxxxxx_battery
```

## Developer notes

web server is part of Mongoose - [https://github.com/cesanta/mongoose](https://github.com/cesanta/mongoose)

## Testing

```bash
# Connect to a mosquitto broker:

mosquitto_sub -h 127.0.0.1 -p 1883 -t "<mqttid>radout/devlist"  # Will display a list of discovered EQ-3 TRVs  
mosquitto_sub -h 127.0.0.1 -p 1883 -t "<mqttid>radout/status/#"  # will show a status message each time a trv is contacted  
mosquitto_pub -h 127.0.0.1 -p 1883 -t "<mqttid>radin/trv/ab:cd:ef:gh:ij:kl/settemp" -m "20.0" # Sets trv temp to 20 degrees
```

## Supported Models

Note: *possible incomplete list because of rebranding eq-3 thermostats*

| Name | Model Name | Factory | Factory Model Number | Remark | Verified |
| ------------- | ------------- | ------------- | ------------- | ------------- | :-------------: |
| Eqiva eQ-3 Bluetooth Smart <sup>2)</sup> | CC-RT-BLE-EQ | EQIVA | 141771E0 / 141771E0A | | X |
| Eqiva eQ-3 Bluetooth Smart (UK Version) <sup>2)</sup> | CC-RT-M-BLE | EQIVA | 142461D0 | | <sup>1)</sup> |
| Eqiva eQ-3 Bluetooth Smart <sup>2)</sup> | | EQIVA | 141771A1A | in the sale | <sup>1)</sup> |
| Eqiva eQ-3 Bluetooth Smart <sup>2)</sup> | CC-RT-M-BLE | EQIVA | 142461A0 | discontinued sales | X |
| EHT CLASSIC MBLE (UK Version) | CC-RT-M-BLE | EQIVA | 142461D0 | | <sup>1)</sup> |
| EHT CLASSIC MBLE (UK Version) | CC-RT-M-BLE | EQIVA | 142461A0 | discontinued sales | <sup>1)</sup> |
| SmartBlue Bluetooth | | | | | <sup>1)</sup> |

<sup>1)</sup> Used the same calor BT-App, so this should work out of the box

<sup>2)</sup> Many aliases for "Eqiva eQ-3 Bluetooth Smart" devices exists. Most of all are characterized by a combination of "Eqiva", "eQ-3", "Bluetooth", "Smart".<br>
&nbsp;&nbsp;&nbsp;&nbsp;e.g. `"eQ-3 AG Eqiva BLUETOOTH® Smart"`, `"eqiva Bluetooth Smart Radiator Thermostat"`, `"eQ-3 eqiva Heizkörperthermostat Typ N"`, `"Eqiva Bluetooth Smart"`

**Don't buy Models without Bluetooth logo. They won't work with this "hub". e.g. "Eqiva Model N, 132231K0A"**

## Credits

- Based on the amazing work of the [ESP-IDF](https://github.com/espressif/esp-idf) project
- and the reverse engineering of [@Heckie75](https://github.com/Heckie75/eQ-3-radiator-thermostat/blob/master/eq-3-radiator-thermostat-api.md)

### Source and continuative reverse engineering by

- Paul ([@softypit](https://github.com/softypit))
- [@ul-gh](https://github.com/ul-gh)
- Peter Becker ([@floyddotnet](https://github.com/floyddotnet))

### Notes

version 1.7 has been ported to use ESP IDF 4.4.3 and Mongoose Embedded Networking Library 7.4.
Various tweaks and bugfixes have been applied including addition of two DNS servers for use in fixed-IP mode and a second NTP server.
A timeout has been added to BLE operations to attempt to prevent the 'freeze-up' experienced occasionally on previous versions.

### Compiling

menuconfig options allow setting of some parameters at compile time.

- The boot-mode GPIO pin (normally a button on GPIO 0 on many ESP32 platforms) used to force the device into AP mode at boot.
- Status LED GPIO which indicates if the device is in AP mode.
- Password for AP mode (enables WPA2PSK) to prevent unwanted access should the device go into AP mode when it is unable to connect to its configured Access Point.

To compile this application, you need to
1. Install ESP-IDF (e.g. via VSCode plugin)
2. go to the IDF directory (e.g. `$HOME/esp/esp-idf`)
3. `git checkout v4.4.3`
4. Run `install.sh` (if this fails, you may need to install Python3.8 and add it explicitly to the search script at `$HOME/esp/esp-idf/tools/detect_python.sh` and delete any previous venv)
5. Go to your project directory, activate the IDF using `. $HOME/esp/esp-idf/export.sh`
6. Run `idf.py build`

When flashing after compilation, please note that the resulting files are located at `build/ota_data_initial.bin`, `build/bootloader/bootloader.bin`, `build/esp32_mqtt_eq3.bin` and `build/partition_table/partition-table.bin`.

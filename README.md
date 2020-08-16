# Arduino MQTT_Mailer Library
<https://github.com/JChristensen/MQTT_Mailer>  
README file  

## License
Arduino MQTT_Mailer Library Copyright (C) 2020 Jack Christensen GNU GPL v3.0

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License v3.0 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/gpl.html>

## Description
A library to send email via MQTT. Derived from the PubSubClient library. Works in conjunction with the mqttmail.py program in the extras directory. The mqttmail.py program and the Mosquitto MQTT broker can run on a Raspberry Pi.

**Prerequisites**  

- Mosquitto MQTT broker, <https://mosquitto.org/>

- Nick O'Leary's PubSubClient library, <https://pubsubclient.knolleary.net/>, <https://github.com/knolleary/pubsubclient>

- Mikal Hart's Streaming library, <http://arduiniana.org/libraries/streaming/>


## Installation
To use the **MQTT_Mailer** library:

- Go to https://github.com/JChristensen/MQTT_Mailer, download the code as a ZIP file and save it to a convenient location on your PC.
- Uncompress the downloaded file.  This will result in a folder containing all the files for the library, that has a name that includes the branch name, usually **MQTT_Mailer-master**.
- Rename the folder to just **MQTT_Mailer**.
- Copy the renamed folder to the Arduino **sketchbook/libraries** folder.

## Extras
- **mqttmail.py:** Python program that runs as an MQTT subscriber. It receives the messages sent by the MQTT_Mailer library and sends them as an email. Run `mqttmail.py --help` for details.
- **mqttmail.conf:** Configuration file for mqttmail.py. See the comments in the file.
- **mqttmail.service:** Sample systemd service file to run mqttmail.py as a service.

## Examples
- **mqttdemo:** Example sketch for Arduino Uno with Ethernet shield. Pressing a button sends an email.


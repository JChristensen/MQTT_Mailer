// Arduino MQTT_Mailer Library
// https://github.com/JChristensen/MQTT_Mailer
// Copyright (C) 2020 by Jack Christensen and licensed under
// GNU GPL v3.0, https://www.gnu.org/licenses/gpl.html
//
// Example sketch for Arduino Uno with Ethernet shield.
// Pushing the button publishes a message intended for the
// mqttmail.py program, which sends it as an email.

#include "heartbeat.h"      // part of this sketch
#include <Ethernet.h>       // standard arduino library
#include <JC_Button.h>      // https://github.com/JChristensen/JC_Button
#include <MQTT_Mailer.h>    // https://github.com/JChristensen/MQTT_Mailer
#include <SPI.h>            // standard arduino library
#include <Streaming.h>      // http://arduiniana.org/libraries/streaming/

// pin assignments
const uint8_t
    SD_SS_PIN(4),           // SD slave select on Ethernet shield
    HB_LED_PIN(8),          // heartbeat led. connect from this pin to ground with an appropriate dropping resistor.
    BUTTON_PIN(9);          // push button to send an email. connect from this pin to ground.

// other constants
const char emailTo[] = "you@email.com";             // email address to send to
const char mqttBroker[] = "pi";                     // mqtt broker hostname
const char clientID[] = "arduino";                  // unique ID for this client
const char pubTopic[] = "sendmail";                 // mqtt publish topic
uint8_t macAddr[] = { 0x02, 0, 0, 0, 0, 0x42 };

// object instantiations
Button btn(BUTTON_PIN);
Heartbeat hbLED(HB_LED_PIN, 50, 1950);
EthernetClient ethClient;
MQTT_Mailer mailer(ethClient, clientID);

void setup()
{
    pinMode(SD_SS_PIN, OUTPUT);     // not using the SD card on the Ethernet shield
    digitalWrite(SD_SS_PIN, HIGH);
    hbLED.begin();
    btn.begin();
    Serial.begin(115200);
    Serial << F( "\n" __FILE__ " " __DATE__ " " __TIME__ "\n" );

    if (Ethernet.begin(macAddr) == 0) {
        Serial << millis() << F(" DHCP fail, press reset!\n");
        while (1);
    }
    delay(1000);    // some time for the Ethernet shield to initialize
    Serial << millis() << F(" Ethernet started ") << Ethernet.localIP() << endl;
    mailer.begin(mqttBroker, 1883, pubTopic);
}

void loop()
{
    static char emailSubj[40], emailText[80];   // must be static
    static bool lastConnected;

    btn.read();
    if (btn.wasReleased()) {
        sprintf(emailSubj, "[%s] Hello, MQTT world!", clientID);
        sprintf(emailText, "At the tone, the \"\"millis\"\" will be %ld.\n-- %s", millis(), clientID);
        mailer.sendmail(emailTo, emailSubj, emailText);
    }
    bool connected = mailer.run();
    if (connected != lastConnected) {
        lastConnected = connected;
        if (connected) {
            hbLED.setInterval(1000, 1000);  // long blink if connected
        }
        else {
            hbLED.setInterval(50, 1950);    // short blink if not connected
        }
    }
    hbLED.run();
}

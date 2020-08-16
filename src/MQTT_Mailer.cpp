// Arduino MQTT_Mailer Library
// A library to send emails via the mqttmail.py program.
// Derived from the PubSubClient class.
// https://github.com/JChristensen/MQTT_Mailer
// Copyright (C) 2020 by Jack Christensen and licensed under
// GNU GPL v3.0, https://www.gnu.org/licenses/gpl.html

#include <MQTT_Mailer.h>

void MQTT_Mailer::begin(const char* mqttBroker, uint16_t port, const char *topic)
{
    m_pubTopic = topic;
    setServer(mqttBroker, 1883);
}

void MQTT_Mailer::sendmail(const char *to, const char *subj, const char *text)
{
    m_to = to;
    m_subj = subj;
    m_text = text;
    m_pubFlag = true;
}

// run the state machine. returns true if connected to the broker.
bool MQTT_Mailer::run()
{
    switch(m_state) {
        case CONNECT:
            if (connect(m_clientID)) {
                m_state = WAIT;
                Serial << millis() << F(" Connected to MQTT broker\n");
            }
            else {
                m_state = WAIT_CONNECT;
                Serial << millis() << F(" Failed to connect to MQTT broker, rc=") << state() << endl;
                Serial << millis() << F(" Retry in ") << m_connectRetry << F(" seconds.\n");
                m_msLastConnect = millis();
            }
            break;

        case WAIT_CONNECT:
            if (millis() - m_msLastConnect >= m_connectRetry * 1000) {
                m_state = CONNECT;
            }
            break;

        case WAIT:
            if (connected()) {
                loop();
                if (m_pubFlag) {
                    m_state = PUBLISH;
                }
            }
            else {
                m_state = CONNECT;
                Serial << millis() << F(" Lost connection to MQTT broker\n");
            }
            break;

        case PUBLISH:
            m_state = WAIT;
            m_pubFlag = false;
            Serial << millis() << F(" MQTT publish: \"") << m_to << F("\",\"") << m_subj << F("\",\"") << m_text << '"' << endl;
            uint16_t lenPayload = strlen(m_to) + strlen(m_subj) + strlen(m_text) + 8;   // 6 quotes + 2 commas
            beginPublish(m_pubTopic, lenPayload, false);
            print('"');
            print(m_to);
            print("\",\"");
            print(m_subj);
            print("\",\"");
            print(m_text);
            print('"');
            endPublish();
            break;
    }
    return (m_state == WAIT || m_state == PUBLISH);
}

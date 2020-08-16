// Arduino MQTT_Mailer Library
// A library to send emails via the mqttmail.py program.
// Derived from the PubSubClient class.
// https://github.com/JChristensen/MQTT_Mailer
// Copyright (C) 2020 by Jack Christensen and licensed under
// GNU GPL v3.0, https://www.gnu.org/licenses/gpl.html

#ifndef MQTT_MAILER_H_INCLUDED
#define MQTT_MAILER_H_INCLUDED
#include <Arduino.h>
#include <PubSubClient.h>
#include <Streaming.h>          // http://arduiniana.org/libraries/streaming/

class MQTT_Mailer : public PubSubClient
{
    enum m_states_t {CONNECT, WAIT_CONNECT, WAIT, PUBLISH};
    public:
        MQTT_Mailer(Client& client, const char *clientID)
            : PubSubClient(client), m_connectRetry(10), m_clientID(clientID) {}
        void begin(const char* server, uint16_t port, const char *topic);
        void setTopic(const char *topic) {m_pubTopic = topic;}
        void sendmail(const char *to, const char *subj, const char *text);
        bool run();

    private:
        m_states_t m_state;
        uint32_t m_connectRetry;        // connect retry interval, seconds
        uint32_t m_msLastConnect;
        const char
            *m_clientID,                // unique ID required for each client
            *m_pubTopic,                // the topic to publish to
            *m_to,                      // email address
            *m_subj,                    // email subject
            *m_text;                    // email body text
        bool m_pubFlag;                 // ready to publish
};
#endif

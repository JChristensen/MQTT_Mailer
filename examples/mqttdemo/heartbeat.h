// Heartbeat LED class
class Heartbeat
{
public:
    Heartbeat(uint8_t pin, uint32_t interval)
        : m_pin(pin), m_onTime(interval), m_offTime(interval), m_state(true) {}
    Heartbeat(uint8_t pin, uint32_t onTime, uint32_t offTime)
        : m_pin(pin), m_onTime(onTime), m_offTime(offTime), m_state(true) {}
    void begin();
    void run();
    void setInterval(uint32_t onTime, uint32_t offTime)
        {m_onTime = onTime; m_offTime = offTime;}

private:
    uint8_t m_pin;
    uint32_t m_onTime;
    uint32_t m_offTime;
    uint32_t m_lastHB;
    bool m_state;
};

void Heartbeat::begin()
{
    pinMode(m_pin, OUTPUT);
    digitalWrite(m_pin, m_state);
    m_lastHB = millis();
}

void Heartbeat::run()
{
    switch (m_state) {
        case true:
            if ( millis() - m_lastHB >= m_onTime ) {
                m_state = false;
                m_lastHB = millis();
                digitalWrite(m_pin, m_state);
            }
            break;

        case false:
            if ( millis() - m_lastHB >= m_offTime ) {
                m_state = true;
                m_lastHB = millis();
                digitalWrite(m_pin, m_state);
            }
            break;
    }
}

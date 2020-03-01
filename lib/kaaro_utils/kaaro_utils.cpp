#include <Arduino.h>

class KaaroUtils
{
public:
    static String getMacAddress()
    {
        uint8_t baseMac[6];
        esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
        uint8_t *hardwareMacPtr = (uint8_t *)baseMac;

        uint32_t hwMacStringReady = 0;

        hwMacStringReady |= hardwareMacPtr[2] << 24;
        hwMacStringReady |= hardwareMacPtr[3] << 16;
        hwMacStringReady |= hardwareMacPtr[4] << 8;
        hwMacStringReady |= hardwareMacPtr[5];

        hwMacStringReady += 1;
        Serial.println(hwMacStringReady);

        return String(hwMacStringReady);
    }
    static uint32_t stoi(String payload, int len)
    {
        uint32_t i = 0;
        uint32_t result = 0;
        for (i = 0; i < len; i++)
        {
            result *= 10;
            result += (char)payload[i] - '0';
        }

        return result;
    }
};
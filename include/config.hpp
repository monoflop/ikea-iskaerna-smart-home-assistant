/*
 * MIT License
 *
 * Copyright (c) 2023 Philipp Kutsch
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef CONFIG_H
#define CONFIG_H

#define JSON_BUFFER_SIZE 512

#include <Arduino.h>
#include "LittleFS.h"
#include <ArduinoJson.h>

/**
 * @brief Configuration model
 */
class Config
{
public:
    String ssid;
    String pass;
    String mqttServer;
    int mqttPort;
    String mqttUser;
    String mqttPass;
    String mqttHaDiscoveryTopicPrefix;
    String mqttHaUniqueId;

    Config(String _ssid,
           String _pass,
           String _mqttServer,
           int _mqttPort,
           String _mqttUser,
           String _mqttPass,
           String _mqttHaDiscoveryTopicPrefix,
           String _mqttHaUniqueId)
    {
        ssid = _ssid;
        pass = _pass;
        mqttServer = _mqttServer;
        mqttPort = _mqttPort;
        mqttUser = _mqttUser;
        mqttPass = _mqttPass;
        mqttHaDiscoveryTopicPrefix = _mqttHaDiscoveryTopicPrefix;
        mqttHaUniqueId = _mqttHaUniqueId;
    }

    /**
     * @brief Load JSON config file from LittleFs and return model
     *
     * ! No sanity checks
     *
     * @param fileName Filename
     * @return Config* Config pointer
     */
    static Config *load(String fileName)
    {
        if (!LittleFS.begin())
        {
            Serial.printf("An Error has occurred while mounting LittleFS\n");
            for (;;)
            {
                delay(100);
            }
        }

        File file = LittleFS.open(fileName, "r");
        if (!file)
        {
            Serial.printf("No config file '%s' found\n", fileName.c_str());
            for (;;)
            {
                delay(100);
            }
        }

        uint8_t payloadString[file.size()] = "";
        file.read(payloadString, file.size());
        file.close();
        Serial.println((char *)payloadString);

        StaticJsonDocument<JSON_BUFFER_SIZE> data;
        DeserializationError err = deserializeJson(data, payloadString);
        if (err)
        {
            Serial.printf("JSON parse error: %s\n", err.c_str());
            for (;;)
            {
                delay(100);
            }
        }

        return new Config(data["wifi_ssid"],
                          data["wifi_pass"],
                          data["mqtt_server"],
                          data["mqtt_port"],
                          data["mqtt_user"],
                          data["mqtt_pass"],
                          data["mqtt_ha_discovery_topic_prefix"],
                          data["mqtt_ha_unique_id"]);
    }
};

#endif
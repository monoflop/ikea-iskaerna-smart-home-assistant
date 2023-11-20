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

#include "network.hpp"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

void NetworkClient::setup()
{
    delay(10);
    Serial.printf("Running WiFi setup\n");
    WiFi.mode(WIFI_STA);

    Serial.printf("Running MQTT setup\n");

    mqttClient = new PubSubClient(wifiClient);
    mqttClient->setBufferSize(MQTT_PACKET_BUFFER_SIZE);
    mqttClient->setServer(config->mqttServer.c_str(), config->mqttPort);
    mqttClient->setCallback(callback);
}

void NetworkClient::connect(String willTopic, uint8_t willQos, boolean willRetain, String willMessage)
{
    // Connect to wifi if not connected
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.printf("WiFi Connecting to %s", config->ssid.c_str());
        WiFi.begin(config->ssid, config->pass);
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            Serial.print(".");
        }
        randomSeed(micros());
        Serial.printf("\nWiFi connected\n");
    }

    if (!mqttClient->connected())
    {
        Serial.printf("MQTT Connecting to %s:%d\n", config->mqttServer.c_str(), config->mqttPort);

        // Loop until we are connected
        while (!mqttClient->connected())
        {
            // Connect
            if (mqttClient->connect(
                    config->mqttServer.c_str(),
                    config->mqttUser.c_str(),
                    config->mqttPass.c_str(),
                    willTopic.c_str(),
                    willQos,
                    willRetain,
                    willMessage.c_str()))
            {
                Serial.printf("MQTT connected\n");
            }
            else
            {
                Serial.printf("Mqtt connection failed. state: %d retring after 5s\n", mqttClient->state());
                delay(5000);
            }
        }
    }
}

bool NetworkClient::isConnected()
{
    return WiFi.status() == WL_CONNECTED && mqttClient->connected();
}

void NetworkClient::loop()
{
    mqttClient->loop();
}

void NetworkClient::subscribe(String topic)
{
    mqttClient->subscribe(topic.c_str());
}

void NetworkClient::publish(String topic, String payload)
{
    mqttClient->publish(topic.c_str(), payload.c_str());
}
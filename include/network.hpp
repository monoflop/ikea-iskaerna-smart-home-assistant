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

#ifndef NETWORK_H
#define NETWORK_H

#include "config.hpp"

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define MQTT_PACKET_BUFFER_SIZE 2048

class NetworkClient
{
private:
    Config *config;
    std::function<void(char *, uint8_t *, unsigned int)> callback;
    WiFiClient wifiClient;
    PubSubClient *mqttClient;

public:
    /**
     * @brief Abstract wrapper for WiFi and MQTT
     */
    NetworkClient(Config *_config, std::function<void(char *, uint8_t *, unsigned int)> _callback)
    {
        config = _config;
        callback = _callback;
    }

    /**
     * @brief Network setup
     */
    void setup();

    /**
     * @brief Connect wifi and mqtt
     *
     * @param willTopic MQTT will topic
     * @param willQos MQTT will quality of service
     * @param willRetain MQTT retain message
     * @param willMessage MQTT will message
     */
    void connect(String willTopic, uint8_t willQos, boolean willRetain, String willMessage);

    /**
     * @brief Is wifi and mqtt connected
     *
     * @return true
     * @return false
     */
    bool isConnected();

    /**
     * @brief Network setup
     */
    void loop();

    /**
     * @brief Subscribe to MQTT topic
     *
     * @param topic Target topic
     */
    void subscribe(String topic);

    /**
     * @brief Publish MQTT message
     *
     * @param topic Target topic
     * @param payload Payload
     */
    void publish(String topic, String payload);
};

#endif
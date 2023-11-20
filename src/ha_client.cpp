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

#include "ha_client.hpp"

#include <ArduinoJson.h>

void HaClient::setup()
{
    networkClient->setup();
}

void HaClient::loop()
{
    if (networkClient->isConnected())
    {
        networkClient->loop();
    }
    // Try to connect and setup with Home Assistant
    else
    {
        // Connect and set last will
        networkClient->connect(availabilityTopic, 1, false, "offline");

        // Subscribe to topics and send discover message
        networkClient->subscribe(haStatusTopic);
        networkClient->subscribe(commandTopic);
        networkClient->subscribe(brightnessCommandTopic);
        networkClient->subscribe(rgbCommandTopic);
        networkClient->subscribe(effectCommandTopic);
        networkClient->publish(
            discoveryTopic,
            discoveryMessage);

        // Notify that we are online
        networkClient->publish(
            availabilityTopic,
            "online");

        // Publish current lamp state
        bool state = getToggleState();
        networkClient->publish(
            stateTopic,
            String(state ? "ON" : "OFF"));

        int brightness = getBrightness();
        networkClient->publish(
            brightnessStateTopic,
            String(brightness));

        int r, g, b;
        std::tie(r, g, b) = getColor();
        networkClient->publish(
            rgbStateTopic,
            String(String(r) + "," + String(g) + "," + String(b)));

        String effect = getEffect();
        networkClient->publish(
            effectStateTopic,
            effect);
    }
}

void HaClient::mqttCallback(char *topic, byte *payload, unsigned int length)
{
    Serial.printf("mqttCallback %s received %d bytes payload: %.*s\n", topic, length, length, payload);
    char payloadStringBuff[length + 1] = "";
    strncpy(payloadStringBuff, (const char *)payload, length);
    payloadStringBuff[length] = '\0';
    String topicString = String(topic);
    String payloadString = String(payloadStringBuff);

    // Home Assistant birth message. Resend device discovery
    if (topicString == haStatusTopic && payloadString == "online")
    {
        Serial.printf("HA is online again\n");
        networkClient->publish(
            discoveryTopic,
            discoveryMessage);
    }
    // Request lamp turn on
    else if (topicString == commandTopic && payloadString == "ON")
    {
        onToggleState(true);
        networkClient->publish(
            stateTopic,
            String("ON"));
    }
    // Request lamp turn off
    else if (topicString == commandTopic && payloadString == "OFF")
    {
        onToggleState(false);
        networkClient->publish(
            stateTopic,
            String("OFF"));
    }
    // Alter brightness
    else if (topicString == brightnessCommandTopic)
    {
        int brightness = atoi(payloadString.c_str());
        onSetBrightness(brightness);
        networkClient->publish(
            brightnessStateTopic,
            String(payloadString));
    }
    // Change rbg color
    else if (topicString == rgbCommandTopic)
    {
        // Rgb payload is following format: 25,44,255
        char *strings[3];
        char *ptr = NULL;
        byte index = 0;
        ptr = strtok((char *)payloadString.c_str(), ",");
        while (ptr != NULL)
        {
            strings[index] = ptr;
            index++;
            ptr = strtok(NULL, ",");
        }

        int r = atoi(strings[0]);
        int g = atoi(strings[1]);
        int b = atoi(strings[2]);
        onSetColor(r, g, b);
        networkClient->publish(
            rgbStateTopic,
            String(payloadString));
    }
    // Change current effect
    else if (topicString == effectCommandTopic)
    {
        onSetEffect(String(payloadString));
    }
}
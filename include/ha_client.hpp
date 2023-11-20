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

#ifndef HA_CLIENT_H
#define HA_CLIENT_H

#include "config.hpp"
#include "network.hpp"

#include <tuple>

class HaClient
{
private:
    // Topics
    const String stateTopic = "iskaerna/smart/light/status";
    const String commandTopic = "iskaerna/smart/light/switch";
    const String availabilityTopic = "iskaerna/smart/light/availability";
    const String brightnessStateTopic = "iskaerna/smart/brightness/status";
    const String brightnessCommandTopic = "iskaerna/smart/brightness/set";
    const String rgbStateTopic = "iskaerna/smart/rgb/status";
    const String rgbCommandTopic = "iskaerna/smart/rgb/set";
    const String effectStateTopic = "iskaerna/smart/effect/status";
    const String effectCommandTopic = "iskaerna/smart/effect/set";

    Config *config;
    NetworkClient *networkClient;
    String haStatusTopic;
    String discoveryTopic;
    String discoveryMessage;

    /**
     * @brief Handle incomming MQTT messages
     *
     * @param topic Incomming topic
     * @param payload Incomming payload
     * @param length Incomming payload byte count
     */
    void mqttCallback(char *topic, byte *payload, unsigned int length);

    std::function<bool()> getToggleState;
    std::function<int()> getBrightness;
    std::function<std::tuple<int, int, int>()> getColor;
    std::function<String()> getEffect;

    std::function<void(bool)> onToggleState;
    std::function<void(int)> onSetBrightness;
    std::function<void(int, int, int)> onSetColor;
    std::function<void(String)> onSetEffect;

public:
    /**
     * @brief Client abstraction for Home Assistant
     *
     * @param _config Config file
     * @param _getToggleState Getter for current light state
     * @param _getBrightness Getter for brightness
     * @param _getColor Getter for rgb color
     * @param _getEffect Getter for current effect
     * @param _onToggleState State event callback
     * @param _onSetBrightness Brightness change callback
     * @param _onSetColor Color change callback
     * @param _onSetEffect Effect change callback
     */
    HaClient(Config *_config,
             std::function<bool()> _getToggleState,
             std::function<int()> _getBrightness,
             std::function<std::tuple<int, int, int>()> _getColor,
             std::function<String()> _getEffect,
             std::function<void(bool)> _onToggleState,
             std::function<void(unsigned int)> _onSetBrightness,
             std::function<void(int, int, int)> _onSetColor,
             std::function<void(String)> _onSetEffect)
    {
        config = _config;
        getToggleState = _getToggleState;
        getBrightness = _getBrightness;
        getColor = _getColor;
        getEffect = _getEffect;
        onToggleState = _onToggleState;
        onSetBrightness = _onSetBrightness;
        onSetColor = _onSetColor;
        onSetEffect = _onSetEffect;
        networkClient = new NetworkClient(_config, [this](char *topic, byte *payload, unsigned int length)
                                          { mqttCallback(topic, payload, length); });

        haStatusTopic = String(config->mqttHaDiscoveryTopicPrefix + "/status");
        discoveryTopic = String(config->mqttHaDiscoveryTopicPrefix + String("/light/") + String(config->mqttHaUniqueId + "/config"));

        // Disconver message
        DynamicJsonDocument json(1024);
        json["unique_id"] = config->mqttHaUniqueId;
        json["name"] = "Ikea Iskaerna Smart";
        json["state_topic"] = stateTopic;
        json["command_topic"] = commandTopic;
        json["availability_topic"] = availabilityTopic;
        json["brightness_state_topic"] = brightnessStateTopic;
        json["brightness_command_topic"] = brightnessCommandTopic;
        json["rgb_state_topic"] = rgbStateTopic;
        json["rgb_command_topic"] = rgbCommandTopic;
        json["effect_state_topic"] = effectStateTopic;
        json["effect_command_topic"] = effectCommandTopic;
        JsonArray ports = json.createNestedArray("effect_list");
        ports.add("none");
        ports.add("rainbow");
        ports.add("pulse");
        json["state_value_template"] = "{{ value_json.state }}";
        json["availability_template"] = "{{ value }}";
        json["brightness_value_template"] = "{{ value_json.brightness }}";
        json["rgb_value_template"] = "{{ value_json.rgb | join(',') }}";
        json["effect_command_template"] = "{{ value }}";
        json["qos"] = 0;
        json["payload_on"] = "ON";
        json["payload_off"] = "OFF";
        json["optimistic"] = true;
        serializeJson(json, discoveryMessage);
    }

    /**
     * @brief Client setup
     *
     * Please call inside your main setup.
     */
    void setup();

    /**
     * @brief Client loop
     *
     * - Checks connection state and reconnects if needed
     * - Setup with Home Assistant
     *
     * Please call inside your main loop.
     */
    void loop();
};

#endif
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

#include "LittleFS.h"
#include <FastLED.h>
#include <tuple>

#include "config.hpp"
#include "ha_client.hpp"

// Number of ws2812b leds
#define NUM_LEDS 6

// LED data pin
#define DATA_PIN 5

// Temporary color store to save latest color information
// once the lamp turns off/on
CRGB savedColor;

// Current LED colors
CRGB leds[NUM_LEDS];

// Active effect
String currentEffect = "none";

//  Rainbow effect current color
int effectColorR = 0;
int effectColorG = 0;
int effectColorB = 0;

// Pulse effect brightness
int effectBrightness = 0;
bool effectBrightnessIncrease = true;

// Home Assistant client
HaClient *client;

// Getter functions for current lamp state
bool getToggleState()
{
  return leds[0] != CRGB::Black;
}

int getBrightness()
{
  return FastLED.getBrightness();
}

std::tuple<int, int, int> getColor()
{
  return std::make_tuple(leds[0].r, leds[0].g, leds[0].b);
}

String getEffect()
{
  return currentEffect;
}

// Callback functions to alter current lamp state
void onToggleState(bool state)
{
  Serial.printf("Toggle lamp state to %d\n", state);
  CRGB target = state ? savedColor : CRGB::Black;

  if (!state)
  {
    savedColor = leds[0];
  }
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = target;
  }
}

void onSetBrightness(int brightness)
{
  FastLED.setBrightness(brightness);
}

void onSetColor(int r, int g, int b)
{
  savedColor.setRGB(r, g, b);
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i].setRGB(r, g, b);
  }
}

void onSetEffect(String effect)
{
  currentEffect = effect;
  if (currentEffect == "rainbow")
  {
    Serial.printf("Starting effect %s\n", effect.c_str());
    effectColorR = leds[0].r;
    effectColorG = leds[0].g;
    effectColorB = leds[0].b;
  }
  else if (currentEffect == "pulse")
  {
    Serial.printf("Starting effect %s\n", effect.c_str());
    effectBrightness = FastLED.getBrightness();
    effectBrightnessIncrease = false;
  }
}

void setup()
{
  delay(500);
  Serial.begin(9600);
  delay(500);

  // Setup FastLED
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  savedColor = CRGB::White;
  for (int i = 0; i < NUM_LEDS; i++)
  {
    // Initially all LEDs are off
    leds[i] = CRGB::Black;
  }
  FastLED.show();

  // Load config and setup Home Assistant client
  Config *config = Config::load("/config.json");
  client = new HaClient(config, getToggleState, getBrightness, getColor, getEffect, onToggleState, onSetBrightness, onSetColor, onSetEffect);
  client->setup();
}

void loop()
{
  // Rainbow effect is enabled
  if (currentEffect == "rainbow")
  {
    // Apply color
    for (int i = 0; i < NUM_LEDS; i++)
    {
      leds[i].setRGB(effectColorR, effectColorG, effectColorB);
    }
    // There is probably a better way to rotate through all rgb colors like using hsv
    if (effectColorR == 255 && effectColorG < 255 && effectColorB == 0)
    {
      effectColorG++;
    }
    else if (effectColorR > 0 && effectColorG == 255 && effectColorB == 0)
    {
      effectColorR--;
    }
    else if (effectColorR == 0 && effectColorG == 255 && effectColorB < 255)
    {
      effectColorB++;
    }
    else if (effectColorR == 0 && effectColorG > 0 && effectColorB == 255)
    {
      effectColorG--;
    }
    else if (effectColorR < 255 && effectColorG == 0 && effectColorB == 255)
    {
      effectColorR++;
    }
    else if (effectColorR == 255 && effectColorG == 0 && effectColorB > 0)
    {
      effectColorB--;
    }
  }
  // Pulse effect is enabled
  else if (currentEffect == "pulse")
  {
    if (effectBrightness == 0)
    {
      effectBrightnessIncrease = true;
    }
    else if (effectBrightness == 255)
    {
      effectBrightnessIncrease = false;
    }
    effectBrightness = effectBrightness + (effectBrightnessIncrease ? 3 : -3);
    FastLED.setBrightness(effectBrightness);
  }

  // Apply fastled changes
  FastLED.show();

  // Loop client
  client->loop();

  delay(50);
}
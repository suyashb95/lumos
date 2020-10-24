#include "SoulDots.h"
#include <ArduinoJson.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // Lumos Service
#define CHARACTERISTIC_UUID_RX "9d1f0205-5b69-46a2-9ff9-d4aec843061a"
#define CHARACTERISTIC_UUID_TX "1f2dbd21-8b5f-46a8-88b4-d8b41158fe2d"

#define NUM_LEDS 60

BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;

bool deviceConnected = false;
bool oldDeviceConnected = false;

uint8_t txValue = 0;

const char *BEHAVIOR_KEY = "behavior";
const char *BRIGHTNESS_KEY = "brightness";
const char *COLORS_KEY = "colors";
const char *ANCHOR_POINTS_KEY = "anchorPoints";
const char *MOTION_RATE_KEY = "motionRate";

SoulDots soulDots;

void setBehavior(DynamicJsonDocument &pattern_config)
{
  const char *behavior = pattern_config[BEHAVIOR_KEY];
  if (!strcmp(behavior, "STATIC"))
  {
    soulDots.set_behavior(STATIC);
  }
  else if (strcmp(behavior, "JUMP") == 0)
  {
    soulDots.set_behavior(FLASH);
  }
  else if (strcmp(behavior, "FADE") == 0)
  {
    soulDots.set_behavior(FADE);
  }
  else if (strcmp(behavior, "WAVE") == 0)
  {
    soulDots.set_behavior(WAVE);
  }
  else
  {
    soulDots.set_behavior(STATIC);
  }
}

void setColors(DynamicJsonDocument &pattern_config)
{
  JsonArray new_colors = pattern_config[COLORS_KEY];
  JsonArray new_anchor_points = pattern_config[ANCHOR_POINTS_KEY];

  CRGB *colors = new CRGB[new_colors.size()];
  uint8_t *anchor_points = NULL;

  for (uint8_t i = 0; i < new_colors.size(); i++)
  {
    int temp = new_colors[i];
    uint8_t red = (temp >> 16) & 0xFF;
    uint8_t green = (temp >> 8) & 0xFF;
    uint8_t blue = temp & 0xFF;
    colors[i] = CRGB(red, green, blue);
  }

  if (new_anchor_points.size() != 0)
  {
    anchor_points = new uint8_t[new_anchor_points.size()];
    for (uint8_t i = 0; i < new_colors.size(); i++)
    {
      anchor_points[i] = new_anchor_points[i];
    }
  }

  soulDots.set_colors(colors, anchor_points, new_colors.size(), new_anchor_points.size());
}

void setBrightness(DynamicJsonDocument &pattern_config)
{
  soulDots.set_max_brightness(pattern_config[BRIGHTNESS_KEY]);
}

void setMotionRate(DynamicJsonDocument &pattern_config)
{
  soulDots.set_animation_rate(pattern_config[MOTION_RATE_KEY]);
}

void updateSoulDots(DynamicJsonDocument &pattern_config)
{
  setColors(pattern_config);
  setBrightness(pattern_config);
  setMotionRate(pattern_config);
  setBehavior(pattern_config);
}

class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer *pServer)
  {
    deviceConnected = false;
  }
};

class MyCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0)
    {
      DynamicJsonDocument patternConfig(1024);
      DeserializationError err = deserializeJson(patternConfig, rxValue.c_str());
      Serial.println(rxValue.c_str());
      switch (err.code())
      {
      case DeserializationError::Ok:
        Serial.println("Deserialization succeeded");
        updateSoulDots(patternConfig);
        soulDots.switch_behavior(&soulDots);
        break;
      case DeserializationError::InvalidInput:
        Serial.println("Invalid input!");
        break;
      case DeserializationError::NoMemory:
        Serial.println("Not enough memory");
        break;
      default:
        Serial.println("Deserialization failed");
        break;
      }
    }
  }
};

void setup()
{
  Serial.begin(115200);
  soulDots.begin(60);

  BLEDevice::init("Lumos Service");
  BLEDevice::setMTU(517);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_RX,
      BLECharacteristic::PROPERTY_WRITE);

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->addServiceUUID(pService->getUUID());
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}

void loop()
{
  if (!deviceConnected && oldDeviceConnected)
  {
    delay(500);                  // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }

  if (deviceConnected && !oldDeviceConnected)
  {
    oldDeviceConnected = deviceConnected;
  }
  soulDots.loop();
}

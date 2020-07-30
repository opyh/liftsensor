#ifndef SETTINGS_H
#define SETTINGS_H

#include "Particle.h"
#include <string>


class Settings {
public:
  typedef struct {
    uint8_t magicValue;
    uint8_t version;
    char equipmentInfoId[20];
    char sourceId[20];
    char token[20];
  } SettingsRecord;

  static void store();
  static SettingsRecord& getFromEEPROM();
  static void setupEEPROMFunctions();
private:
  static int storeEquipmentInfoId(String equipmentInfoId);
  static int storeSourceId(String sourceId);
  static int storeToken(String token);
  static Settings::SettingsRecord currentSettings;

  static const uint8_t MagicValue;
};

#endif
#include "Settings.h"

Settings::SettingsRecord Settings::currentSettings = {};
const uint8_t Settings::MagicValue = 0xf4;

void Settings::store() {
    currentSettings.magicValue = MagicValue;
    currentSettings.version = 1;
    EEPROM.put(0, currentSettings);
}

Settings::SettingsRecord &Settings::getFromEEPROM() {
    EEPROM.get(0, currentSettings);
    return currentSettings;
}

int Settings::storeEquipmentInfoId(String equipmentInfoId) {
    getFromEEPROM();
    strcpy(currentSettings.equipmentInfoId, equipmentInfoId.c_str());
    store();
    return 1;
}

int Settings::storeSourceId(String sourceId) {
    getFromEEPROM();
    strcpy(currentSettings.sourceId, sourceId.c_str());
    store();
    return 1;
}

int Settings::storeToken(String token) {
    getFromEEPROM();
    strcpy(currentSettings.token, token.c_str());
    store();
    return 1;
}

void Settings::setupEEPROMFunctions() {
    Particle.function("setId", storeEquipmentInfoId);
    Particle.function("setSourceId", storeSourceId);
    Particle.function("setToken", storeToken);

    getFromEEPROM();

    if (currentSettings.magicValue != MagicValue) {
        memset(&currentSettings, 0, sizeof(SettingsRecord));
        currentSettings.magicValue = MagicValue;
        currentSettings.version = 1;
    }
}
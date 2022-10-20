#include <EEPROM.h>
#include "config.h"
#include "eeprom_state.h"
#include "memory.h"

uint32_t Memory::GetAgnosticChipId()
{
#if defined(ARDUINO_ARCH_ESP8266)
    return ESP.getChipId();
#elif defined(ESP32)
    uint32_t id = 0;
    for (int i = 0; i < 17; i = i + 8)
    {
        id |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }
    return id;
#else
    return 0;
#endif
}

bool Memory::VerifyEEPROMValidity()
{
    EEPROMState state;
    EEPROM.get(0, state);
    if (memcmp(state.init_string, EEPROM_INITSTRING, sizeof(EEPROM_INITSTRING)) != 0)
    {
        Serial.println("State is invalid! Initstring did not match");
        Serial.println(state.init_string);
        return false;
    }
    return true;
}

bool Memory::InitializeEEPROM()
{
    EEPROMState state;
    state.vault_locked = false;
    strcpy(state.init_string, EEPROM_INITSTRING);
    state.servo_closed_position = DEFAULT_SERVO_CLOSED_POSITION;
    state.servo_open_position = DEFAULT_SERVO_OPEN_POSITION;
    char default_name[EEPROM_MAX_NAME_LENGTH];
    sprintf(default_name, "%s%06X", DEFAULT_BOX_NAME_PREFIX, this->GetAgnosticChipId());
    strcpy(state.name, default_name);
    EEPROM.put(0, state);

    return EEPROM.commit();
}

Memory::Memory()
{
    EEPROM.begin(sizeof(EEPROMState));
    if (!this->VerifyEEPROMValidity())
    {
        if (!this->InitializeEEPROM())
        {
            Serial.println("Could not initialize EEPROM!");
            ESP.restart();
        }
    }
}

bool Memory::SetName(const char *name)
{
    if (strlen(name) >= EEPROM_MAX_NAME_LENGTH + 1)
    {
        return false;
    }
    EEPROMState state;
    EEPROM.get(0, state);
    strcpy(state.name, name);
    EEPROM.put(0, state);
    return EEPROM.commit();
}

int Memory::GetName(char *name, int len)
{
    EEPROMState state;
    EEPROM.get(0, state);
    int copied = strlcpy(name, state.name, len);
    return copied;
}

bool Memory::SetOpenPosition(int position)
{
    EEPROMState state;
    EEPROM.get(0, state);
    state.servo_open_position = position;
    EEPROM.put(0, state);
    return EEPROM.commit();
}
bool Memory::SetClosedPosition(int position)
{
    EEPROMState state;
    EEPROM.get(0, state);
    state.servo_closed_position = position;
    EEPROM.put(0, state);
    return EEPROM.commit();
}

int Memory::GetOpenPosition()
{
    EEPROMState state;
    EEPROM.get(0, state);
    return state.servo_open_position;
}

int Memory::GetClosedPosition()
{
    EEPROMState state;
    EEPROM.get(0, state);
    return state.servo_closed_position;
}

bool Memory::SetVaultLocked(const char *new_password)
{
    if (strlen(new_password) >= EEPROM_MAX_PASSWORD_LENGTH + 1)
    {
        return false;
    }
    EEPROMState state;
    EEPROM.get(0, state);
    if (state.vault_locked)
    {
        return false;
    }
    strcpy(state.vault_password, new_password);
    state.vault_locked = true;
    EEPROM.put(0, state);
    return EEPROM.commit();
}

bool Memory::SetVaultUnlocked(const char *password)
{
    EEPROMState state;
    EEPROM.get(0, state);
    if (strcmp(state.vault_password, password) == 0)
    {
        state.vault_locked = false;
        EEPROM.put(0, state);
        return EEPROM.commit();
    }
    else
    {
        return false;
    }
}

bool Memory::GetVaultIsLocked()
{
    EEPROMState state;
    EEPROM.get(0, state);
    return state.vault_locked;
}

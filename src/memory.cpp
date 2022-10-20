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
    return EEPROM.commit();
}
bool Memory::SetClosedPosition(int position)
{
    EEPROMState state;
    EEPROM.get(0, state);
    state.servo_closed_position = position;
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
    return EEPROM.commit();
}

bool Memory::SetVaultUnlocked(char *password)
{
    EEPROMState state;
    EEPROM.get(0, state);
    if (strcmp(state.vault_password, password) == 0)
    {
        state.vault_locked = false;
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

/*

bool set_software_locked(bool lock)
{
    EEPROMStateObject state;
    EEPROM.get(EEPROM_STATE_ADDR, state);
    state.locked = lock;
    EEPROM.put(EEPROM_STATE_ADDR, state);
    if (EEPROM.commit())
    {
        Serial.println("set lock status eeprom committed");
        return true;
    }
    else
    {
        Serial.println("set lock status eeprom commit failed!");
        return false;
    }
}

bool get_password(char *buf)
{
    EEPROMPasswordObject password_object;
    EEPROM.get(EEPROM_PASSWORD_ADDR, password_object);
    strcpy(buf, password_object.password);
    return true;
}

bool set_password(const char *newPassword)
{
    EEPROMPasswordObject password_object;
    EEPROM.get(EEPROM_PASSWORD_ADDR, password_object);
    strcpy(password_object.password, newPassword);
    EEPROM.put(EEPROM_PASSWORD_ADDR, password_object);
    if (EEPROM.commit())
    {
        Serial.println("password eeprom committed");
        return true;
    }
    else
    {
        Serial.println("password eeprom commit failed!");
        return false;
    }
}

bool get_is_locked()
{
    EEPROMStateObject state;
    state.locked = false;
    EEPROM.get(EEPROM_STATE_ADDR, state);
    if (state.locked)
    {
        return true;
    }
    else
    {
        return false;
    }
}

*/

#include <string.h>
#include <ArduinoJson.h>
#include <config.h>
#include "lockbox.h"
#include "memory.h"

Lockbox::Lockbox(Lock* lock, Memory* memory) : lock(lock), memory(memory)
{
}

set_password_result Lockbox::SetVaultLocked(const char* key)
{
    if (this->memory->GetVaultIsLocked())
    {
        return ALREADY_LOCKED;
    }
    else
    {
        if (this->memory->SetVaultLocked(key))
        {
            return PASSWORD_OK;
        }
        else
        {
            return PASSWORD_INTERNAL_ERROR;
        }
    }
}

set_password_result Lockbox::SetVaultUnlocked(const char* key)
{
    if (!this->memory->GetVaultIsLocked())
    {
        return ALREADY_UNLOCKED;
    }
    else
    {
        char stored_password[64];
        this->memory->GetVaultPassword((char*)&stored_password, 64);
        if (strcmp(key, stored_password) == 0)
        {
            return PASSWORD_OK;
        }
        else
        {
            return WRONG_PASSWORD;
        }
    }
}

set_settings_result Lockbox::SetBoxName(const char* name)
{
    this->memory->SetName(name);
    return SETTINGS_OK;
}

set_settings_result Lockbox::SetServoOpenPosition(int position)
{
    if (this->memory->GetVaultIsLocked())
    {
        return LOCKED;
    }
    else
    {
        this->memory->SetOpenPosition(position);
        return SETTINGS_OK;
    }
}

set_settings_result Lockbox::SetServoClosedPosition(int position)
{
    if (this->memory->GetVaultIsLocked())
    {
        return LOCKED;
    }
    else
    {
        this->memory->SetClosedPosition(position);
        return SETTINGS_OK;
    }
}

bool Lockbox::FactoryReset()
{
    if (this->memory->GetVaultIsLocked())
    {
        return false;
    }
    else
    {
        this->memory->Reset();
        return true;
    }
}

void Lockbox::ForceFactoryReset()
{
    this->memory->Reset();
}

bool Lockbox::GetVaultLocked()
{
    return this->memory->GetVaultIsLocked();
}

bool Lockbox::GetSettings(DynamicJsonDocument* doc)
{
    (*doc)["locked"] = memory->GetVaultIsLocked();
    (*doc)["servo_open_position"] = memory->GetOpenPosition();
    (*doc)["servo_closed_position"] = memory->GetClosedPosition();
    (*doc)["version"] = FIRMWARE_VERSION;
    char name[EEPROM_MAX_NAME_LENGTH];
    this->memory->GetName(name, EEPROM_MAX_NAME_LENGTH);
    (*doc)["name"] = name;
    return true;
}

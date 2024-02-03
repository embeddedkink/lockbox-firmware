#include <ArduinoJson.h>
#include <config.h>
#include "lockbox.h"
#include "memory.h"

Lockbox::Lockbox(Lock *lock, Memory *memory) : lock(lock), memory(memory)
{
    if (this->memory->GetVaultIsLocked() || this->memory->GetVaultIsEmlalocked())
    {
        this->lock->SetClosed();
    }
    else
    {
        this->lock->SetOpen();
    }

    this->emlalock_incleaning = false;
}

set_password_result Lockbox::SetVaultLocked(const char *key)
{
    if (this->memory->GetVaultIsLocked())
    {
        return ALREADY_LOCKED;
    }
    else
    {
        if (strlen(key) <= MAX_PASSWORD_LENGTH && this->memory->SetVaultLocked(key))
        {
            this->lock->SetClosed();
            return PASSWORD_OK;
        }
        else
        {
            return PASSWORD_INTERNAL_ERROR;
        }
    }
}

set_password_result Lockbox::SetVaultUnlocked(const char *key)
{
    if (!this->memory->GetVaultIsLocked())
    {
        return ALREADY_UNLOCKED;
    }
    else
    {
        char stored_password[MAX_PASSWORD_LENGTH + 1];
        this->memory->GetVaultPassword(stored_password, sizeof(stored_password));
        if (strcmp(key, stored_password) == 0)
        {
            this->memory->SetVaultUnlocked();
            if (!this->memory->GetVaultIsLocked() && !this->memory->GetVaultIsEmlalocked())
                this->lock->SetOpen();
            return PASSWORD_OK;
        }
        else
        {
            return WRONG_PASSWORD;
        }
    }
}

set_settings_result Lockbox::SetBoxName(const char *name)
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

set_settings_result Lockbox::SetEmlalockApiUser(const char *apiUser)
{
    if (this->memory->GetVaultIsLocked())
    {
        return LOCKED;
    }
    else
    {
        this->memory->SetEmlalockApiUser(apiUser);
        return SETTINGS_OK;
    }
}

set_settings_result Lockbox::SetEmlalockApiKey(const char *apiKey)
{
    if (this->memory->GetVaultIsLocked())
    {
        return LOCKED;
    }
    else
    {
        this->memory->SetEmlalockApiKey(apiKey);
        return SETTINGS_OK;
    }
}

set_password_result Lockbox::SetVaultEmlalocked(const char *key)
{
    if (this->memory->GetVaultIsEmlalocked())
    {
        return ALREADY_LOCKED;
    }
    else
    {
        if (strlen(key) <= MAX_PASSWORD_LENGTH && this->memory->SetVaultEmlalocked(key))
        {
            this->lock->SetClosed();
            return PASSWORD_OK;
        }
        else
        {
            return PASSWORD_INTERNAL_ERROR;
        }
    }
}

set_password_result Lockbox::SetVaultUnemlalocked()
{
    if (!this->memory->GetVaultIsEmlalocked())
    {
        return ALREADY_UNLOCKED;
    }
    else
    {
        if (this->memory->SetVaultUnemlalocked() && !this->memory->GetVaultIsLocked())
            this->lock->SetOpen();
        return PASSWORD_OK;
    }
}

bool Lockbox::GetVaultEmlalocked()
{
    return this->memory->GetVaultIsEmlalocked();
}

void Lockbox::SetVaultEmlalockIncleaning(bool state)
{
    this->emlalock_incleaning = state;
    if (state)
        this->lock->SetOpen();
    else
        this->lock->SetClosed();
}

bool Lockbox::GetVaultEmlalockIncleaning()
{
    return this->emlalock_incleaning;
}

bool Lockbox::GetSettings(DynamicJsonDocument *doc)
{
    (*doc)["locked"] = this->memory->GetVaultIsLocked();
    (*doc)["emlalocked"] = this->memory->GetVaultIsEmlalocked();
    (*doc)["incleaning"] = this->emlalock_incleaning;
    (*doc)["servo_open_position"] = this->memory->GetOpenPosition();
    (*doc)["servo_closed_position"] = this->memory->GetClosedPosition();
    (*doc)["version"] = FIRMWARE_VERSION;
    char name[32];
    this->memory->GetName(name, 32);
    (*doc)["name"] = name;
    char emlalock_api_user[32];
    this->memory->GetEmlalockApiUser(emlalock_api_user, 32);
    (*doc)["emlalock_api_user"] = emlalock_api_user;
    char emlalock_api_key[32];
    this->memory->GetEmlalockApiKey(emlalock_api_key, 32);
    (*doc)["emlalock_api_key"] = emlalock_api_key;

    return true;
}

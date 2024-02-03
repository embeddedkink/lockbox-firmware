#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "config.h"
#include "memory.h"

uint32_t Memory::GetAgnosticChipId()
{
#if defined(ESP8266)
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

bool Memory::LoadString(char const *path, char const *setting, char *value, int len)
{
    DynamicJsonDocument doc(1024);
    File file = LittleFS.open(path, "r");
    if (file == 0)
    {
        return false;
    }
    deserializeJson(doc, file);
    file.close();

    strncpy(value, doc[setting], len);
    return true;
}

int Memory::LoadInt(char const *path, char const *setting)
{
    DynamicJsonDocument doc(1024);
    File file = LittleFS.open(path, "r");
    deserializeJson(doc, file);
    file.close();
    return doc[setting];
}

bool Memory::Save(char const *path, char const *setting, char const *value)
{
    DynamicJsonDocument doc(1024);
    File file = LittleFS.open(path, "r");
    if (file == 0)
    {
        return false;
    }
    deserializeJson(doc, file);
    file.close();

    doc[setting] = value;

    file = LittleFS.open(path, "w");
    if (file == 0)
    {
        return false;
    }
    serializeJson(doc, file);
    file.close();
    return true;
}

bool Memory::Save(char const *path, char const *setting, int value)
{
    DynamicJsonDocument doc(1024);
    File file = LittleFS.open(path, "r");
    if (file == 0)
    {
        return false;
    }
    deserializeJson(doc, file);
    file.close();

    doc[setting] = value;

    file = LittleFS.open(path, "w");
    if (file == 0)
    {
        return false;
    }
    serializeJson(doc, file);
    file.close();
    return true;
}

Memory::Memory()
{
    // LittleFS.begin();
    char name[MAX_NAME_LENGTH] = "a";
    LoadString("/settings.json", "name", name, MAX_NAME_LENGTH);
    if (strlen(name) == 0)
    {
        char default_name[MAX_NAME_LENGTH];
        sprintf(default_name, "%s%06X", DEFAULT_BOX_NAME_PREFIX, this->GetAgnosticChipId());
        Save("settings.json", "name", default_name);
    }
}

bool Memory::SetName(const char *name)
{
    return Save("/settings.json", "name", name);
}

bool Memory::GetName(char *name, int len)
{
    return LoadString("/settings.json", "name", name, len);
}

bool Memory::SetOpenPosition(int position)
{
    return Save("/settings.json", "open_position", position);
}

bool Memory::SetClosedPosition(int position)
{
    return Save("/settings.json", "closed_position", position);
}

int Memory::GetOpenPosition()
{
    return LoadInt("/settings.json", "open_position");
}

int Memory::GetClosedPosition()
{
    return LoadInt("/settings.json", "closed_position");
}

bool Memory::SetVaultLocked(const char *new_password)
{
    return Save("/state.json", "password", new_password);
}

bool Memory::SetVaultUnlocked()
{
    return Save("/state.json", "password", "");
}

bool Memory::GetVaultIsLocked()
{
    char password[MAX_PASSWORD_LENGTH + 1];
    LoadString("/state.json", "password", password, sizeof(password));
    if (strlen(password) == 0)
        return false;
    return true;
}

bool Memory::SetVaultEmlalocked(const char *new_password)
{
    return Save("/state.json", "emlalock_sessionid", new_password);
}

bool Memory::SetVaultUnemlalocked()
{
    return Save("/state.json", "emlalock_sessionid", "");
}

bool Memory::GetVaultIsEmlalocked()
{
    char sessionid[32];
    LoadString("/state.json", "emlalock_sessionid", sessionid, sizeof(sessionid));
    if (strlen(sessionid) == 0)
        return false;
    return true;
}

void Memory::GetVaultPassword(char *password, int length)
{
    LoadString("/state.json", "password", password, length);
}

bool Memory::SetEmlalockApiUser(const char *api_user)
{
    return Save("/settings.json", "emlalock_api_user", api_user);
}

bool Memory::SetEmlalockApiKey(const char *api_key)
{
    return Save("/settings.json", "emlalock_api_key", api_key);
}

bool Memory::SetEmlalockSessionid(const char *sessionid)
{
    return Save("/state.json", "emlalock_sessionid", sessionid);
}

bool Memory::GetEmlalockApiUser(char *api_user, int len)
{
    return LoadString("/settings.json", "emlalock_api_user", api_user, len);
}

bool Memory::GetEmlalockApiKey(char *api_key, int len)
{
    return LoadString("/settings.json", "emlalock_api_key", api_key, len);
}

bool Memory::GetEmlalockSessionid(char *sessionid, int len)
{
    return LoadString("/state.json", "emlalock_sessionid", sessionid, len);
}

void Memory::Reset()
{
    Save("/settings.json", "name", "");
    Save("/settings.json", "emlalock_api_user", "");
    Save("/settings.json", "emlalock_api_key", "");
    Save("/state.json", "password", "");
    Save("/state.json", "emlalock_sessionid", "");
}

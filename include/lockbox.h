#ifndef LOCKBOX_H
#define LOCKBOX_H

#include <ArduinoJson.h>
#include "lock.h"
#include "memory.h"
#include "lockbox_result.h"

class Lockbox
{
private:
    Lock *lock;
    Memory *memory;
    bool emlalock_incleaning;

public:
    Lockbox(Lock *lock, Memory *memory);
    set_password_result SetVaultLocked(const char *key);
    set_password_result SetVaultUnlocked(const char *key);
    set_settings_result SetBoxName(const char *name);
    set_settings_result SetServoOpenPosition(int position);
    set_settings_result SetServoClosedPosition(int position);
    set_settings_result SetEmlalockApiUser(const char *apiUser);
    set_settings_result SetEmlalockApiKey(const char *apiKey);
    bool FactoryReset();
    void ForceFactoryReset();
    bool GetVaultLocked();
    bool GetSettings(DynamicJsonDocument *doc);
    set_password_result SetVaultEmlalocked(const char *key);
    set_password_result SetVaultUnemlalocked();
    bool GetVaultEmlalocked();
    void SetVaultEmlalockIncleaning(bool state);
    bool GetVaultEmlalockIncleaning();
};

#endif // LOCKBOX_H

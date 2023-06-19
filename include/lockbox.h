#ifndef LOCKBOX_H
#define LOCKBOX_H

#include <ArduinoJson.h>
#include "lock.h"
#include "memory.h"
#include "lockbox_result.h"

class Lockbox
{
private:
    Lock* lock;
    Memory* memory;
public:
    Lockbox(Lock* lock, Memory* memory);
    set_password_result SetVaultLocked(const char* key);
    set_password_result SetVaultUnlocked(const char* key);
    set_settings_result SetBoxName(const char* name);
    set_settings_result SetServoOpenPosition(int position);
    set_settings_result SetServoClosedPosition(int position);
    bool FactoryReset();
    void ForceFactoryReset();
    bool GetVaultLocked();
    bool GetSettings(DynamicJsonDocument* doc);
};

#endif // LOCKBOX_H

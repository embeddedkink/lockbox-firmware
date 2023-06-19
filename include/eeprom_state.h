#ifndef EEPROM_STATE_H
#define EEPROM_STATE_H

#define EEPROM_INITSTRING "EKI_LOCKBOX"

#include "config.h"

struct EEPROMState
{
    char init_string[sizeof(EEPROM_INITSTRING)];
    char name[EEPROM_MAX_NAME_LENGTH];
    int servo_closed_position;
    int servo_open_position;
    bool vault_locked;
    char vault_password[EEPROM_MAX_PASSWORD_LENGTH];
};

#endif // EEPROM_STATE_H

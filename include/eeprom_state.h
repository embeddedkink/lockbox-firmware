#ifndef EEPROM_STATE_H
#define EEPROM_STATE_H

#define EEPROM_MAX_PASSWORD_LENGTH 64
#define EEPROM_MAX_NAME_LENGTH 32
#define EEPROM_INITSTRING "EKI_LOCKBOX"

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

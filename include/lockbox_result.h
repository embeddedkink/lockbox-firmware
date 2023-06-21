#ifndef LOCKBOX_RESULT_H
#define LOCKBOX_RESULT_H

enum set_password_result
{
    PASSWORD_OK,
    ALREADY_LOCKED,
    ALREADY_UNLOCKED,
    WRONG_PASSWORD,
    PASSWORD_INTERNAL_ERROR
};

enum set_settings_result
{
    SETTINGS_OK,
    LOCKED,
    SETTINGS_INTERNAL_ERROR
};

#endif // LOCKBOX_RESULT_H

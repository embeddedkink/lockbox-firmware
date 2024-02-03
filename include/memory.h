#pragma once

#define DEFAULT_BOX_NAME_PREFIX "eki_lockbox_"

class Memory
{
private:
    uint32_t GetAgnosticChipId();
    bool LoadString(char const *path, char const *setting, char *value, int len);
    int LoadInt(char const *path, char const *setting);
    bool Save(char const *path, char const *setting, char const *value);
    bool Save(char const *path, char const *setting, int value);

public:
    Memory();

    bool SetName(const char *name);
    bool GetName(char *name, int len);

    bool SetOpenPosition(int position);
    bool SetClosedPosition(int position);
    int GetOpenPosition();
    int GetClosedPosition();

    bool SetVaultLocked(const char *new_password);
    bool SetVaultUnlocked();
    bool GetVaultIsLocked();
    void GetVaultPassword(char *password, int length);

    bool SetVaultEmlalocked(const char *new_password);
    bool SetVaultUnemlalocked();
    bool GetVaultIsEmlalocked();
    bool SetEmlalockApiUser(const char *api_user);
    bool SetEmlalockApiKey(const char *api_key);
    bool SetEmlalockSessionid(const char *sessionid);
    bool GetEmlalockApiUser(char *api_user, int len);
    bool GetEmlalockApiKey(char *api_key, int len);
    bool GetEmlalockSessionid(char *sessionid, int len);

    void Reset();
};

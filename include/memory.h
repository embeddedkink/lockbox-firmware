#pragma once

#define DEFAULT_BOX_NAME_PREFIX "eki_lockbox_"

class Memory
{
private:
    uint32_t GetAgnosticChipId();
    bool LoadString(char* path, char* setting, char* value, int len);
    int LoadInt(char* path, char* setting);
    bool Save(char* path, char* setting, char* value);
    bool Save(char* path, char* setting, int value);
public:
    Memory();

    bool SetName(const char *name);
    int GetName(char *name, int len);

    bool SetOpenPosition(int position);
    bool SetClosedPosition(int position);
    int GetOpenPosition();
    int GetClosedPosition();

    bool SetVaultLocked(const char *new_password);
    bool SetVaultUnlocked();
    bool GetVaultIsLocked();
    void GetVaultPassword(char* password, int length);

    void Reset();
};

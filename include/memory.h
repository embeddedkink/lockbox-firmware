#pragma once

class Memory
{
private:
    bool InitializeEEPROM();
    uint32_t GetAgnosticChipId();
    bool VerifyEEPROMValidity();
public:
    Memory();

    bool SetName(const char *name);
    int GetName(char *name, int len);

    bool SetOpenPosition(int position);
    bool SetClosedPosition(int position);
    int GetOpenPosition();
    int GetClosedPosition();

    bool SetVaultLocked(const char *new_password);
    bool SetVaultUnlocked(const char *password);
    bool GetVaultIsLocked();

    void Reset();
};

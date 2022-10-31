#include <AsyncJson.h>
#include <ArduinoJson.h>
#include "api.h"
#include "config.h"
#include "eeprom_state.h"
#include "lock.h"
#include "main.h"
#include "memory.h"


void StartServer()
{
    api_server->onNotFound(NotFound);
    api_server->on("/lock", HTTP_POST, ActionLock);
    api_server->on("/unlock", HTTP_POST, ActionUnlock);
    api_server->on("/settings", HTTP_GET, ActionSettingsGet);
    api_server->on("/settings", HTTP_POST, ActionSettingsPost);
    api_server->on("/reset", HTTP_POST, ActionReset);
    api_server->begin();
}

void NotFound(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->setCode(404);
    DynamicJsonDocument doc(512);
    doc["result"] = "error";
    doc["error"] = "NotFound";
    serializeJson(doc, *response);
    request->send(response);
}

void ActionLock(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Access-Control-Allow-Origin", "*");
    DynamicJsonDocument doc(512);

    if (memory->GetVaultIsLocked())
    {
        response->setCode(400);
        doc["result"] = "error";
        doc["error"] = "AlreadyLocked";
    }
    else
    {
        String password;
        if (request->hasParam("password", true))
        {
            password = request->getParam("password", true)->value();
            bool saved_lock = memory->SetVaultLocked(password.c_str());
            if (saved_lock)
            {
                lock->SetClosed();
                response->setCode(200);
                doc["result"] = "success";
            }
            else
            {
                response->setCode(500);
                doc["result"] = "error";
                doc["error"] = "MemoryError";
            }
        }
        else
        {
            response->setCode(400);
            doc["result"] = "error";
            doc["error"] = "NoPassword";
        }
    }
    serializeJson(doc, *response);
    request->send(response);
}

void ActionUnlock(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Access-Control-Allow-Origin", "*");
    DynamicJsonDocument doc(512);

    String password;
    if (request->hasParam("password", true))
    {
        password = request->getParam("password", true)->value();
        bool saved_unlock = memory->SetVaultUnlocked(password.c_str());
        if (saved_unlock)
        {
            lock->SetOpen();
            response->setCode(200);
            doc["result"] = "success";
        }
        else
        {
            response->setCode(500);
            doc["result"] = "error";
            doc["error"] = "PasswordFailure";
        }
    }
    else
    {
        response->setCode(400);
        doc["result"] = "error";
        doc["error"] = "NoPassword";
    }

    serializeJson(doc, *response);
    request->send(response);
}

void ActionSettingsGet(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Access-Control-Allow-Origin", "*");
    DynamicJsonDocument doc(512);
    response->setCode(200);
    doc["result"] = "succes";
    doc["data"]["locked"] = memory->GetVaultIsLocked();
    doc["data"]["servo_open_position"] = memory->GetOpenPosition();
    doc["data"]["servo_closed_position"] = memory->GetClosedPosition();
    doc["data"]["version"] = FIRMWARE_VERSION;
    char name[EEPROM_MAX_NAME_LENGTH];
    memory->GetName(name, EEPROM_MAX_NAME_LENGTH);
    doc["data"]["name"] = name;
    serializeJson(doc, *response);
    request->send(response);
}

void ActionSettingsPost(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Access-Control-Allow-Origin", "*");
    DynamicJsonDocument doc(1024);

    bool setting_updated = false;
    if (memory->GetVaultIsLocked())
    {
        response->setCode(400);
        doc["result"] = "error";
        doc["error"] = "VaultLocked";
    }
    else
    {

        String name;
        if (request->hasParam("name", true))
        {
            name = request->getParam("name", true)->value();
            bool saved = memory->SetName(name.c_str());
            if (saved)
            {
                response->setCode(200);
                doc["result"] = "success";
                setting_updated = true;
            }
            else
            {
                response->setCode(500);
                doc["result"] = "error";
                doc["error"] = "MemoryError";
            }
        }

        if (request->hasParam("servo_open_position", true))
        {
            int open_position = request->getParam("servo_open_position", true)->value().toInt();
            if (memory->SetOpenPosition(open_position))
            {
                response->setCode(200);
                doc["result"] = "success";
                setting_updated = true;
            }
            else
            {
                response->setCode(500);
                doc["result"] = "error";
                doc["error"] = "MemoryError";
            }
        }

        if (request->hasParam("servo_closed_position", true))
        {
            int closed_position = request->getParam("servo_closed_position", true)->value().toInt();
            if (memory->SetClosedPosition(closed_position))
            {
                response->setCode(200);
                doc["result"] = "success";
                setting_updated = true;
            }
            else
            {
                response->setCode(500);
                doc["result"] = "error";
                doc["error"] = "MemoryError";
            }
        }

        if (!setting_updated)
        {
            response->setCode(400);
            doc["result"] = "error";
            doc["error"] = "NotSaved";
        }
    }

    serializeJson(doc, *response);
    request->send(response);

    // HACK
    if (setting_updated)
    {
        ESP.restart();
    }
}

void ActionReset(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Access-Control-Allow-Origin", "*");
    DynamicJsonDocument doc(512);

    if (memory->GetVaultIsLocked())
    {
        response->setCode(400);
        doc["result"] = "error";
        doc["error"] = "VaultLocked";
        serializeJson(doc, *response);
        request->send(response);
    }
    else
    {
        memory->Reset();
        wifiManager->resetSettings();
        response->setCode(200);
        doc["result"] = "success";
        serializeJson(doc, *response);
        request->send(response);
        ESP.restart();
    }
}

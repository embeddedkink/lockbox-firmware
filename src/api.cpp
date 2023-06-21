#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <ESPAsyncWiFiManager.h>
#include "api.h"
#include "lockbox.h"
#include "lockbox_result.h"

Lockbox* api_lockbox;
AsyncWiFiManager* api_wifiManager;

void StartServer(AsyncWebServer* api_server, Lockbox* lockbox, AsyncWiFiManager* wifiManager)
{
    api_lockbox = lockbox;
    api_wifiManager = wifiManager;
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

    String password;
    if (request->hasParam("password", true))
    {
        password = request->getParam("password", true)->value();
        set_password_result result = api_lockbox->SetVaultLocked(password.c_str());
        if (result == PASSWORD_OK)
        {
            response->setCode(200);
            doc["result"] = "success";
        }
        else if (result == ALREADY_LOCKED)
        {
            response->setCode(401);
            doc["result"] = "error";
            doc["error"] = "AlreadyLocked";
        }
        else
        {
            response->setCode(500);
            doc["result"] = "error";
            doc["error"] = "UnexpectedError";
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

void ActionUnlock(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Access-Control-Allow-Origin", "*");
    DynamicJsonDocument doc(512);

    String password;
    if (request->hasParam("password", true))
    {
        password = request->getParam("password", true)->value();
        set_password_result result = api_lockbox->SetVaultUnlocked(password.c_str());
        if (result == PASSWORD_OK)
        {
            response->setCode(200);
            doc["result"] = "success";
        }
        else if (result == WRONG_PASSWORD)
        {
            response->setCode(401);
            doc["result"] = "error";
            doc["error"] = "WrongPassword";
        }
        else if (result == ALREADY_UNLOCKED)
        {
            response->setCode(400);
            doc["result"] = "error";
            doc["error"] = "AlreadyUnlocked";
        }
        else
        {
            response->setCode(500);
            doc["result"] = "error";
            doc["error"] = "UnexpectedError";
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
    DynamicJsonDocument data(512);
    api_lockbox->GetSettings(&data);
    doc["data"] = data;
    serializeJson(doc, *response);
    request->send(response);
}

void ActionSettingsPost(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Access-Control-Allow-Origin", "*");
    DynamicJsonDocument doc(1024);

    bool setting_updated = false;
    bool setting_failed = false;
    bool setting_not_allowed = false;

    String name;
    if (request->hasParam("name", true))
    {
        name = request->getParam("name", true)->value();
        set_settings_result result = api_lockbox->SetBoxName(name.c_str());
        if (result == SETTINGS_OK)
        {
            setting_updated = true;
        }
        else if (result == LOCKED)
        {
            setting_not_allowed = true;
        }
        else
        {
            setting_failed = true;
        }
    }

    if (request->hasParam("servo_open_position", true))
    {
        int open_position = request->getParam("servo_open_position", true)->value().toInt();
        set_settings_result result = api_lockbox->SetServoOpenPosition(open_position);
        if (result == SETTINGS_OK)
        {
            setting_updated = true;
        }
        else if (result == LOCKED)
        {
            setting_not_allowed = true;
        }
        else
        {
            setting_failed = true;
        }
    }

    if (request->hasParam("servo_closed_position", true))
    {
        int closed_position = request->getParam("servo_closed_position", true)->value().toInt();
        set_settings_result result = api_lockbox->SetServoClosedPosition(closed_position);
        if (result == SETTINGS_OK)
        {
            setting_updated = true;
        }
        else if (result == LOCKED)
        {
            setting_not_allowed = true;
        }
        else
        {
            setting_failed = true;
        }
    }

    if (setting_failed)
    {
        response->setCode(500);
        doc["result"] = "error";
        doc["error"] = "InternalError";
    }
    else
    {
        if (setting_not_allowed)
        {
            response->setCode(401);
            doc["result"] = "error";
            doc["error"] = "VaultLocked";
        }
        else if (setting_updated)
        {
            response->setCode(200);
            doc["result"] = "success";
        }
        else
        {
            response->setCode(500);
            doc["result"] = "error";
            doc["error"] = "UnknownParameter";
        }
    }

    serializeJson(doc, *response);
    request->send(response);
}

void ActionReset(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Access-Control-Allow-Origin", "*");
    DynamicJsonDocument doc(512);

    bool success = api_lockbox->FactoryReset();

    if (!success)
    {
        response->setCode(401);
        doc["result"] = "error";
        doc["error"] = "VaultLocked";
        serializeJson(doc, *response);
        request->send(response);
    }
    else
    {
        response->setCode(200);
        doc["result"] = "success";
        serializeJson(doc, *response);
        request->send(response);
        
        api_wifiManager->resetSettings();
        ESP.restart();
    }
}

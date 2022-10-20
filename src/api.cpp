#include <AsyncJson.h>
#include <ArduinoJson.h>
#include "api.h"
#include "config.h"
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
            doc["error"] = "BadPassword";
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
            doc["error"] = "BadPassword";
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
    doc["data"]["open_position"] = memory->GetOpenPosition();
    doc["data"]["closed_position"] = memory->GetClosedPosition();
    doc["data"]["version"] = FIRMWARE_VERSION;
    serializeJson(doc, *response);
    request->send(response);
}

void ActionSettingsPost(AsyncWebServerRequest *request)
{
}

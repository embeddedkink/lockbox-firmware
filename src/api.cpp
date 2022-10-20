#include "AsyncJson.h"
#include "ArduinoJson.h"
#include "api.h"
#include "lock.h"
#include "memory.h"

#include "main.h"

void StartServer()
{
    api_server->onNotFound(NotFound);
    api_server->on("/lock", HTTP_POST, ActionLock);
    api_server->on("/unlock", HTTP_POST, ActionUnlock);
    api_server->on("/update", HTTP_POST, ActionUpdate);
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
    extern Lock* lock;
    extern Memory* memory;

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Access-Control-Allow-Origin", "*");
    DynamicJsonDocument doc(512);

    String password;
    if (request->hasParam("password", true))
    {
        password = request->getParam("password", true)->value();
        lock->SetOpen();
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
}
void ActionUpdate(AsyncWebServerRequest *request)
{
}
void ActionSettingsGet(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Access-Control-Allow-Origin", "*");
    DynamicJsonDocument doc(512);
    response->setCode(200);
    doc["result"] = "succes";
    doc["error"] = "";
    doc["test"] = memory->GetVaultIsLocked();
    serializeJson(doc, *response);
    request->send(response);
}
void ActionSettingsPost(AsyncWebServerRequest *request)
{
}

/*
void respond_json(AsyncWebServerRequest *request, int code, String jsonString)
{
    AsyncWebServerResponse *response = request->beginResponse(code, "application/json", jsonString);
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
}

void action_lock(AsyncWebServerRequest *request)
{
    if (get_is_locked())
    {
        respond_json(request, 400, "{\"result\":\"error\", \"error\":\"alreadyLocked\"}");
        return;
    }
    String password;
    if (request->hasParam("password", true))
    {
        password = request->getParam("password", true)->value();
    }
    else
    {
        respond_json(request, 400, "{\"result\":\"error\", \"error\":\"noPassword\"}");
        return;
    }
    if (password.length() >= MAX_PASSWORD_LENGTH - 1)
    {
        respond_json(request, 400, "{\"result\":\"error\", \"error\":\"longPassword\"}");
        return;
    }
    Serial.print("Locking with password: ");
    Serial.println(password);
    set_password(password.c_str());
    if (set_software_locked(true))
    {
        set_hardware_locked(true);
        respond_json(request, 200, "{\"result\":\"success\"}");
    }
    else
    {
        respond_json(request, 500, "{\"result\":\"error\", \"error\":\"eepromError\"}");
    }
}

void action_unlock(AsyncWebServerRequest *request)
{
    String password;
    if (request->hasParam("password", true))
    {
        password = request->getParam("password", true)->value();
    }
    else
    {
        respond_json(request, 400, "{\"result\":\"error\", \"error\":\"noPassword\"}");
        return;
    }
    Serial.print("Unlocking with password: ");
    Serial.println(password);
    char savedPassword[MAX_PASSWORD_LENGTH];
    get_password(savedPassword);
    if (strcmp(savedPassword, password.c_str()) == 0)
    {
        set_hardware_locked(false);
        if (set_software_locked(false))
        {
            respond_json(request, 200, "{\"result\":\"success\"}");
        }
        else
        {
            respond_json(request, 500, "{\"result\":\"error\", \"error\":\"eepromError\"}");
        }
    }
    else
    {
        respond_json(request, 400, "{\"result\":\"error\", \"error\":\"wrongPassword\"}");
    }
}

void action_update(AsyncWebServerRequest *request)
{
    if (get_is_locked())
    {
        set_hardware_locked(true);
    }
    else
    {
        set_hardware_locked(false);
    }
    respond_json(request, 200, "{\"result\":\"success\"}");
}

void action_settings_get(AsyncWebServerRequest *request)
{
    EEPROMSettingsObject settings;
    EEPROM.get(EEPROM_SETTINGS_ADDR, settings);
    const int responsebuflen = 200;
    char buf[responsebuflen];
    sprintf(buf, "{\"result\":\"success\", \"data\":{\"locked\":%i,\"servo_closed_position\":%i,\"servo_open_position\":%i,\"name\":\"%s\",\"version\":\"%s\"}}", get_is_locked(), settings.servo_closed_position, settings.servo_open_position, settings.name, FIRMWARE_VERSION);
    respond_json(request, 200, buf);
}

// Currently only supports one attribute per request
void action_settings_post(AsyncWebServerRequest *request)
{
    bool is_locked = get_is_locked();
    EEPROMSettingsObject settings;
    EEPROM.get(EEPROM_SETTINGS_ADDR, settings);
    if (request->hasParam("name", true))
    {
        String name = request->getParam("name", true)->value();
        if (name.length() <= MAX_NAME_LENGTH)
        {
            strcpy(settings.name, name.c_str());
            EEPROM.put(EEPROM_SETTINGS_ADDR, settings);
            EEPROM.commit();
            respond_json(request, 200, "{\"result\":\"success\"}");
        }
        else
        {
            respond_json(request, 500, "{\"result\":\"error\", \"error\":\"invalidParameter\"}");
        }
    }
    else if (request->hasParam("servo_open_position", true))
    {
        if (!is_locked)
        {
            String newpos = request->getParam("servo_open_position", true)->value();
            settings.servo_open_position = newpos.toInt();
            EEPROM.put(EEPROM_SETTINGS_ADDR, settings);
            EEPROM.commit();
            respond_json(request, 200, "{\"result\":\"success\"}");
        }
        else
        {
            respond_json(request, 400, "{\"result\":\"error\", \"error\":\"currentlyLocked\"}");
        }
    }
    else if (request->hasParam("servo_closed_position", true))
    {
        if (!is_locked)
        {
            String newpos = request->getParam("servo_closed_position", true)->value();
            settings.servo_open_position = newpos.toInt();
            EEPROM.put(EEPROM_SETTINGS_ADDR, settings);
            EEPROM.commit();
            respond_json(request, 200, "{\"result\":\"success\"}");
        }
        else
        {
            respond_json(request, 400, "{\"result\":\"error\", \"error\":\"currentlyLocked\"}");
        }
    }
    else
    {
        respond_json(request, 400, "{\"result\":\"error\", \"error\":\"invalidParameter\"}");
    }
}
*/
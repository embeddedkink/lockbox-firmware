#include <Arduino.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <WiFiClient.h>
#include "config.h"

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <Servo.h>
#include "ESP8266mDNS.h"
#elif defined(ESP32)
#include <ESP32Servo.h>
#include "ESPmDNS.h"
#endif

// Due to improper compatibility these 3 must happen in this order:
#include <WiFiManager.h>
#define WEBSERVER_H
#include <ESPAsyncWebServer.h>

#define EEPROM_STATE_ADDR 128
#define EEPROM_PASSWORD_ADDR EEPROM_STATE_ADDR + sizeof(EEPROMStateObject)
#define EEPROM_SETTINGS_ADDR EEPROM_PASSWORD_ADDR + sizeof(EEPROMPasswordObject)
#define EEPROM_SIZE (EEPROM_STATE_ADDR + sizeof(EEPROMStateObject) + sizeof(EEPROMPasswordObject) + sizeof(EEPROMSettingsObject))
#define DEFAULT_SERVO_OPEN_POSITION 180
#define DEFAULT_SERVO_CLOSED_POSITION 0
#define DEFAULT_BOX_NAME_PREFIX "eki_lockbox_"

struct EEPROMStateObject
{
    char initstring[sizeof(INITSTRING)];
    bool locked;
};

struct EEPROMPasswordObject
{
    char password[MAX_PASSWORD_LENGTH];
};

struct EEPROMSettingsObject
{
    char name[MAX_NAME_LENGTH];
    int servo_closed_position;
    int servo_open_position;
};

Servo myservo;
WiFiManager wifiManager;
WiFiClient client;
AsyncWebServer server(TCP_PORT);

uint32_t get_agnostic_chip_id()
{
    #if defined(ARDUINO_ARCH_ESP8266)
    return ESP.getChipId();
    #elif defined(ESP32)
    uint32_t id = 0;
    for(int i=0; i<17; i=i+8) {
    id |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }
    return id;
    #else
    return 0;
    #endif
}

// EEPROM related functions

bool verify_eeprom_state_validity()
{
    EEPROMStateObject state;
    EEPROM.get(EEPROM_STATE_ADDR, state);
    if (memcmp(state.initstring, INITSTRING, sizeof(INITSTRING)) != 0)
    {
        Serial.println("State is invalid! Initstring did not match");
        Serial.println(state.initstring);
        return false;
    }
    return true;
}

bool initialize_eeprom()
{
    EEPROMStateObject state;
    state.locked = false;
    strcpy(state.initstring, INITSTRING);
    EEPROM.put(EEPROM_STATE_ADDR, state);

    EEPROMSettingsObject settings;
    settings.servo_closed_position = DEFAULT_SERVO_CLOSED_POSITION;
    settings.servo_open_position = DEFAULT_SERVO_OPEN_POSITION;
    
    char default_name[MAX_NAME_LENGTH];
    sprintf(default_name, "%s%06X", DEFAULT_BOX_NAME_PREFIX, get_agnostic_chip_id());

    strcpy(settings.name, default_name);
    EEPROM.put(EEPROM_SETTINGS_ADDR, settings);

    if (EEPROM.commit())
    {
        Serial.println("init eeprom committed");
        return true;
    }
    else
    {
        Serial.println("init eeprom commit failed!");
        return false;
    }
}

// State manipulation

bool set_software_locked(bool lock)
{
    EEPROMStateObject state;
    EEPROM.get(EEPROM_STATE_ADDR, state);
    state.locked = lock;
    EEPROM.put(EEPROM_STATE_ADDR, state);
    if (EEPROM.commit())
    {
        Serial.println("set lock status eeprom committed");
        return true;
    }
    else
    {
        Serial.println("set lock status eeprom commit failed!");
        return false;
    }
}

bool get_password(char *buf)
{
    EEPROMPasswordObject password_object;
    EEPROM.get(EEPROM_PASSWORD_ADDR, password_object);
    strcpy(buf, password_object.password);
    return true;
}

bool set_password(const char *newPassword)
{
    EEPROMPasswordObject password_object;
    EEPROM.get(EEPROM_PASSWORD_ADDR, password_object);
    strcpy(password_object.password, newPassword);
    EEPROM.put(EEPROM_PASSWORD_ADDR, password_object);
    if (EEPROM.commit())
    {
        Serial.println("password eeprom committed");
        return true;
    }
    else
    {
        Serial.println("password eeprom commit failed!");
        return false;
    }
}

bool get_is_locked()
{
    EEPROMStateObject state;
    state.locked = false;
    EEPROM.get(EEPROM_STATE_ADDR, state);
    if (state.locked)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void set_hardware_locked(bool lock)
{
    // Can be optimized but setting lock is infrequent anyway
    EEPROMSettingsObject settings;
    EEPROM.get(EEPROM_SETTINGS_ADDR, settings);
    if (lock)
    {
        myservo.write(settings.servo_closed_position);
    }
    else
    {
        myservo.write(settings.servo_open_position);
    }
}

// Request handlers

void respond_json(AsyncWebServerRequest* request, int code, String jsonString)
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
    if (request->hasParam("password", true)) {
        password = request->getParam("password", true)->value();
    } else {
        respond_json(request, 400, "{\"result\":\"error\", \"error\":\"noPassword\"}");
        return;
    }
    if (password.length() >= MAX_PASSWORD_LENGTH -1)
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
    if (request->hasParam("password", true)) {
        password = request->getParam("password", true)->value();
    } else {
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

void notFound(AsyncWebServerRequest *request) {
    respond_json(request, 404, "{\"result\":\"error\", \"error\":\"notFound\"}");
}

void setup()
{
    Serial.begin(9600);
    delay(500);
    EEPROM.begin(EEPROM_SIZE);
    myservo.attach(PINSERVO);

    if (!verify_eeprom_state_validity())
    {
        Serial.println("EEPROM not properly initialized. fixing.");
        if (!initialize_eeprom())
        {
            Serial.println("Could not initialize EEPROM!");
            delay(3000);
            ESP.restart();
            delay(5000);
        }
    }
    else
    {
        Serial.println("\n\nEEPROM valid");
    }

    if (get_is_locked())
    {
        Serial.println("State is locked");
        if (set_software_locked(true))
        {
            set_hardware_locked(true);
        }
        else
        {
            Serial.println("Could not set sw locked");
            set_hardware_locked(false);
        }
    }
    else
    {
        Serial.println("State is unlocked");
        set_hardware_locked(false);
    }

    Serial.print("Password: ");
    char pwd[MAX_PASSWORD_LENGTH];
    get_password(pwd);
    Serial.println(pwd);

    WiFi.softAPdisconnect(true);
    if (!wifiManager.autoConnect("EKI Lockbox"))
    {
        Serial.println("Failed to connect and hit timeout");
        delay(3000);
        ESP.restart();
        delay(5000);
    }

    EEPROMSettingsObject settings;
    EEPROM.get(EEPROM_SETTINGS_ADDR, settings);
    if (!MDNS.begin(settings.name))
    {
        Serial.println("Error setting up MDNS responder!");
    }
    else
    {
        Serial.println("mDNS responder started");
    }
    MDNS.addService("ekilb", "tcp", TCP_PORT);

    server.onNotFound(notFound);
    server.on("/lock", HTTP_POST, action_lock);
    server.on("/unlock", HTTP_POST, action_unlock);
    server.on("/update", HTTP_POST, action_update);
    server.on("/settings", HTTP_GET, action_settings_get);
    server.on("/settings", HTTP_POST, action_settings_post);
    server.begin();
}

void loop()
{
}

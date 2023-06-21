#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include <WiFiClient.h>
#include "api.h"
#include "config.h"
#include "lock.h"
#include "lockbox.h"
#include "memory.h"

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include "ESP8266mDNS.h"
#elif defined(ESP32)
#include <WiFi.h>
#include "ESPmDNS.h"
#endif
#include <ESPAsyncWiFiManager.h>
#include <ESPAsyncWebServer.h>

WiFiClient *client;
DNSServer *dns;
AsyncWebServer *api_server;
AsyncWebServer *frontend_server;
AsyncWiFiManager *wifiManager;
Memory *memory;
Lock *lock;
Lockbox *lockbox;

String api_host;

String processor(const String &var)
{
    if (var == "API_HOST")
        return api_host;
    return String();
}

void setup()
{
    Serial.begin(9600);
    delay(10);

    if (!LittleFS.begin())
    {
        Serial.println("An Error has occurred while mounting LittleFS");
    }

#if defined(ESP8266)
    pinMode(D3, INPUT_PULLUP);
#endif

    client = new WiFiClient;
    dns = new DNSServer;
    api_server = new AsyncWebServer(API_PORT);
    frontend_server = new AsyncWebServer(FRONTEND_PORT);

    memory = new Memory();
    lock = new Lock(PINSERVO, memory->GetOpenPosition(), memory->GetClosedPosition());
    lockbox = new Lockbox(lock, memory);

    WiFi.softAPdisconnect(true);
    char box_name[MAX_NAME_LENGTH];
    memory->GetName(box_name, MAX_NAME_LENGTH);
    wifiManager = new AsyncWiFiManager(frontend_server, dns);
    if (!wifiManager->autoConnect(box_name))
    {
        Serial.println("Failed to connect and hit timeout");
        delay(3000);
        ESP.restart();
    }

    api_host = "";
    api_host.concat("http://");
    api_host.concat(WiFi.localIP().toString());
    api_host.concat(":");
    api_host.concat(API_PORT);

    // Wifi is connected, we can repurpose frontend server
    frontend_server->reset();
    frontend_server->begin();
    frontend_server->serveStatic("/", LittleFS, "/www").setTemplateProcessor(processor);
    ;

    MDNS.addService("ekilb", "tcp", API_PORT);
    if (!MDNS.begin(box_name))
    {
        Serial.println("Error setting up MDNS responder!");
    }
    else
    {
        Serial.println("mDNS responder started");
    }
    StartServer(api_server, lockbox, wifiManager); // api.h
}

void loop()
{
#if defined(ESP8266)
    MDNS.update();
#endif

#if defined(ESP8266)
    if (!digitalRead(D3))
    {
        memory->SetVaultUnlocked();
        ESP.restart();
    }
#endif
}

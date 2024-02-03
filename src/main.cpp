#include <Arduino.h>
#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>
#include <FS.h>
#include <HTTPClient.h>
#include <LittleFS.h>
#include <WiFiClientSecure.h>

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

WiFiClientSecure *client;
HTTPClient https;
DNSServer *dns;
AsyncWebServer *api_server;
AsyncWebServer *frontend_server;
AsyncWiFiManager *wifiManager;
Memory *memory;
Lock *lock;
Lockbox *lockbox;

String api_host;
static unsigned long last_time = 0;

String processor(const String &var)
{
    if (var == "API_HOST")
        return api_host;
    return String();
}

void check_emlalock_session()
{
    char api_user[32];
    char api_key[32];
    char url[100];
    DynamicJsonDocument info(6144);

    memory->GetEmlalockApiUser(api_user, sizeof(api_user));
    memory->GetEmlalockApiKey(api_key, sizeof(api_user));
    sprintf(url, "https://api.emlalock.com/info?userid=%s&apikey=%s", api_user, api_key);

    https.useHTTP10(true);
    https.begin(*client, url);
    https.GET();
    deserializeJson(info, https.getStream());
    https.end();

    const char *error = info["error"];
    if (error != NULL)
    {
        Serial.println(error);
        return;
    }

    bool is_emlalocked = lockbox->GetVaultEmlalocked();
    const char *chastitysessionid = info["chastitysession"]["chastitysessionid"];

    if (is_emlalocked && chastitysessionid == NULL)
    {
        /* no Emlalock session detected, but the vault is emlalocked.
           Unemlalock the vault. */
        lockbox->SetVaultUnemlalocked();
    }
    else if (!is_emlalocked && chastitysessionid != NULL)
    {
        /* sessionid isn't yet stored, time to lock up */
        lockbox->SetVaultEmlalocked(chastitysessionid);
    }
    else if (is_emlalocked && chastitysessionid != NULL)
    {
        /* vault is emlalocked, check if we are in a cleaningopening */
        lockbox->SetVaultEmlalockIncleaning(info["chastitysession"]["incleaning"]);
    }
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

    client = new WiFiClientSecure;
    client->setInsecure();
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
    DefaultHeaders::Instance().addHeader("X-Content-Type-Options", "nosniff");
    DefaultHeaders::Instance().addHeader("Content-Security-Policy", "default-src 'self'; style-src 'self' unpkg.com; script-src 'self';connect-src *;base-uri 'self';form-action 'self'");
    DefaultHeaders::Instance().addHeader("Referrer-Policy", "no-referrer");
    frontend_server->reset();
    frontend_server->begin();
    frontend_server->serveStatic("/", LittleFS, "/www");
    frontend_server->serveStatic("/templates", LittleFS, "/templates").setTemplateProcessor(processor);

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
#elif defined(ESP32)
    if (!digitalRead(0))
#endif
    {
        memory->SetVaultUnlocked();
        ESP.restart();
    }

    if (last_time + 5000 < millis())
    {
        last_time = millis();
        check_emlalock_session();
    }
}

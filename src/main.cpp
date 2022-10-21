#include "main.h"
#include "config.h"
#include "eeprom_state.h"
#include "api.h"

WiFiClient* client;
DNSServer* dns;
AsyncWebServer* api_server;
AsyncWebServer* frontend_server;
AsyncWiFiManager* wifiManager;
Memory* memory;
Lock* lock;

String api_host;

#include <FS.h>
#include <LittleFS.h>

String processor(const String& var)
{
  if(var == "API_HOST")
    return api_host;
  return String();
}

void setup()
{
    Serial.begin(9600);
    delay(500);

    client = new WiFiClient;
    dns = new DNSServer;
    api_server = new AsyncWebServer(API_PORT);
    frontend_server = new AsyncWebServer(80);

    memory = new Memory();
    lock = new Lock(PINSERVO, memory->GetOpenPosition(), memory->GetClosedPosition());
    if (memory->GetVaultIsLocked())
    {
        lock->SetClosed();
    }
    else
    {
        lock->SetOpen();
    }

    WiFi.softAPdisconnect(true);
    char box_name[EEPROM_MAX_NAME_LENGTH];
    memory->GetName(box_name, EEPROM_MAX_NAME_LENGTH);
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
    if(!LittleFS.begin()){
        Serial.println("An Error has occurred while mounting LittleFS");
    }
    frontend_server->serveStatic("/", LittleFS, "/").setTemplateProcessor(processor);;

    MDNS.addService("ekilb", "tcp", API_PORT);
    if (!MDNS.begin(box_name))
    {
        Serial.println("Error setting up MDNS responder!");
    }
    else
    {
        Serial.println("mDNS responder started");
    }
    MDNS.announce();

    StartServer(); // api.h
}

void loop()
{
}

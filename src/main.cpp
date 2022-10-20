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

void setup()
{
    Serial.begin(9600);
    delay(500);

    client = new WiFiClient;
    dns = new DNSServer;
    api_server = new AsyncWebServer(TCP_PORT);
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
    if (!MDNS.begin(box_name))
    {
        Serial.println("Error setting up MDNS responder!");
    }
    else
    {
        Serial.println("mDNS responder started");
    }
    MDNS.addService("ekilb", "tcp", TCP_PORT);

    StartServer(); // api.h
}

void loop()
{
}

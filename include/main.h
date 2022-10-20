#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>
#include <WiFiClient.h>

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include "ESP8266mDNS.h"
#elif defined(ESP32)
#include <WiFi.h>
#include "ESPmDNS.h"
#endif

#include <ESPAsyncWiFiManager.h>
#include <ESPAsyncWebServer.h>

#include "config.h"
#include "lock.h"
#include "memory.h"

extern WiFiClient* client;
extern DNSServer* dns;
extern AsyncWebServer* api_server;
extern AsyncWebServer* frontend_server;
extern AsyncWiFiManager* wifiManager;
extern Memory* memory;
extern Lock* lock;

#endif // MAIN_H

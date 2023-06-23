#ifndef API_H
#define API_H

#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>
#include "lockbox.h"

void StartServer(AsyncWebServer *api_server, Lockbox *lockbox, AsyncWiFiManager *wifiManager);
void NotFound(AsyncWebServerRequest *request);
void ActionLock(AsyncWebServerRequest *request);
void ActionUnlock(AsyncWebServerRequest *request);
void ActionSettingsGet(AsyncWebServerRequest *request);
void ActionSettingsPost(AsyncWebServerRequest *request);
void ActionReset(AsyncWebServerRequest *request);

#endif // API_H

#ifndef API_H
#define API_H

#include <ESPAsyncWebServer.h>

void StartServer();
void ActionLock(AsyncWebServerRequest *request);
void ActionUnlock(AsyncWebServerRequest *request);
void ActionUpdate(AsyncWebServerRequest *request);
void ActionSettingsGet(AsyncWebServerRequest *request);
void ActionSettingsPost(AsyncWebServerRequest *request);
void NotFound(AsyncWebServerRequest *request);
void ActionReset(AsyncWebServerRequest *request);

#endif // API_H

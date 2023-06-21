#define FIRMWARE_VERSION "20230621-dev"

#define MAX_NAME_LENGTH 64
#define MAX_PASSWORD_LENGTH 64

#define API_PORT 5000
#define FRONTEND_PORT 80

#if defined(ESP8266)
#define PINSERVO D4
#elif defined(ESP32)
#define PINSERVO 13
#endif

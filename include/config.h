// Version is set at build time in the pipeline
#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "undefined"
#endif

#define MAX_NAME_LENGTH 64
#define MAX_PASSWORD_LENGTH 64

#define API_PORT 5000

#define DEFAULT_SERVO_OPEN_POSITION 180
#define DEFAULT_SERVO_CLOSED_POSITION 0

#define DEFAULT_BOX_NAME_PREFIX "eki_lockbox_"
#define FRONTEND_PORT 80

#if defined(ESP8266)
#define PINSERVO D4
#elif defined(ESP32)
#define PINSERVO 13
#endif

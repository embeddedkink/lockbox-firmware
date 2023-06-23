<<<<<<< HEAD
// Version is set at build time in the pipeline
#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "ver:dev"
#endif

#define MAX_NAME_LENGTH 64
#define MAX_PASSWORD_LENGTH 64
=======
#define FIRMWARE_VERSION "20230604-dev"
>>>>>>> cd37bea (ux changes, minor api changes)

#define API_PORT 5000

#define DEFAULT_SERVO_OPEN_POSITION 180
#define DEFAULT_SERVO_CLOSED_POSITION 0

#define DEFAULT_BOX_NAME_PREFIX "eki_lockbox_"

#if defined(ESP8266)
#define PINSERVO D4
#elif defined(ESP32)
#define PINSERVO 13
#endif

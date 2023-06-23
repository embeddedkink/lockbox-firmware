#ifndef LOCK_H
#define LOCK_H

#if defined(ESP8266)
#include <Servo.h>
#elif defined(ESP32)
#include <ESP32Servo.h>
#endif

class Lock
{
private:
    Servo servo;
    int open_position;
    int closed_position;

public:
    Lock(int servo_pin, int open_position, int closed_position);
    void SetOpenPosition(int position);
    void SetClosedPosition(int position);
    void SetOpen();
    void SetClosed();
};

#endif // LOCK_H

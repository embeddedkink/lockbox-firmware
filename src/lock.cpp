#include "lock.h"

Lock::Lock(int servo_pin, int open_position, int closed_position) : open_position(open_position), closed_position(closed_position)
{
    this->servo.attach(servo_pin);
}

void Lock::SetOpenPosition(int position) { this->open_position = position; }
void Lock::SetClosedPosition(int position) { this->closed_position = position; }
void Lock::SetOpen() { this->servo.write(this->open_position); }
void Lock::SetClosed() { this->servo.write(this->closed_position); }

#include "Door.h"

#define greenPin 12
#define OPEN_TIME 2000

bool Door::is_open = false;
bool Door::bistable = false;
bool Door::lockdown = false;
uint32_t Door::close_at = 0;

void Door::init() {
    SPI.begin(); 
    pinMode(greenPin, OUTPUT);
}

void Door::tick() {
    if (close_at == 0)
        return;

    if (close_at > millis())
        return;

    close();
}

bool Door::get_bistable() {
    return bistable;
}

bool Door::get_lockdown() {
    return lockdown;
}

void Door::set_bistable(bool value) {
    bistable = value;

    if (value) {
        close_at = 0;
        return;
    }

    if (is_open)
        close();
}

void Door::set_lockdown(bool value) {
    lockdown = value;

    if (!value)
        return;

    if (is_open)
        close();
}

void Door::open(bool override) {
    if (lockdown && !override)
        return;

    if (is_open && bistable) {
        close();
        return;
    }
    
    is_open = true;
    if (!bistable)
        close_at = millis() + OPEN_TIME;
    digitalWrite(greenPin, HIGH);
}

void Door::close() {
    digitalWrite(greenPin, LOW);
    close_at = 0;
    is_open = false;
}
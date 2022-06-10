#pragma once

#include <Arduino.h>
#include <SPI.h>

class Door {
    public:
        static void init();
        static void tick();
        static bool get_bistable();
        static bool get_lockdown();
        static void set_bistable(bool value);
        static void set_lockdown(bool value);
        static void open(bool override);

    private:
        static bool is_open;
        static bool bistable;
        static bool lockdown;
        static uint32_t close_at;
        static void close();
};
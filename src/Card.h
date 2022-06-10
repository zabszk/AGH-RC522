#pragma once

#include "Door.h"
#include "Structs.h"
#include "UART.h"
#include <MFRC522.h>

#define SS_PIN 21
#define RST_PIN 22

#define BLOCK_SIZE 18
#define MAX_CARDS 10

class Card {
    public:
        static void init();
        static void tick();
        static access_card *cards[];
        static bool programming_mode;

    private:
        static uint8_t buffer[];
        static bool process();
        static bool read();
};
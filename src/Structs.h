#pragma once

#include "Arduino.h"

typedef struct access_card_def {
  uint32_t uid;
  uint8_t *sector1;
} access_card;

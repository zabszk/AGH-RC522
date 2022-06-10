#include "Card.h"

MFRC522::MIFARE_Key key;
MFRC522 rfid(SS_PIN, RST_PIN);

bool Card::programming_mode = false;
uint8_t Card::buffer[BLOCK_SIZE];
access_card *Card::cards[MAX_CARDS];

void Card::init() {
    rfid.PCD_Init();

    for (int i = 0; i < MAX_CARDS; i++)
        cards[i] = NULL;

    for (int i = 0; i < 6; i++)
        key.keyByte[i] = 0xFF; //KEY-A
}

void Card::tick() {
    if (process()) {
        rfid.PICC_HaltA();
        rfid.PCD_StopCrypto1();
    }
}

bool Card::process() {
    if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial())
        return false;

    MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
    uint32_t uid = (rfid.uid.uidByte[0] << 24) | (rfid.uid.uidByte[1] << 16) | (rfid.uid.uidByte[2] << 8) | rfid.uid.uidByte[3];

    int i = 0;
    int fn = -1;
    for (; i <= MAX_CARDS; i++) {
        if (i == MAX_CARDS)
            break;

        if (cards[i] == NULL) {
            fn = i;
            continue;
        }
        
        if (cards[i]->uid == uid)
            break;
    }
    
    if (programming_mode) {
        programming_mode = false;

        if (fn == -1) {
            UART::print_error("Cards limit reached!");
            return true;
        }

        if (i != MAX_CARDS) {
            free(cards[i]->sector1);
            free(cards[i]);
            cards[i] = NULL;
            UART::print_success("Card removed.");
            return true;
        }

        if (!read())
            return true;

        cards[fn] = (access_card*) malloc(sizeof(access_card));
        cards[fn]->uid = uid;
        cards[fn]->sector1 = (uint8_t*)malloc(BLOCK_SIZE);
        memcpy(cards[fn]->sector1, buffer, BLOCK_SIZE);

        UART::print_success("Card added.");
        return true;
    }

    if (i == MAX_CARDS) {
        UART::print_error("Access denied (unknown card)!");
        return true;
    }

    if (!read())
        return true;

    if (memcmp(buffer, cards[i]->sector1, BLOCK_SIZE) != 0) {
        UART::print_error("Access denied (invalid sector 1)!");
        return true;
    }

    UART::print_success("Access granted.");
    Door::open(false);
    return true;
}

bool Card::read() {
    MFRC522::StatusCode status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 1, &key, &(rfid.uid));
    if (status != MFRC522::STATUS_OK) {
        UART::print_error("Authentication failed!");
        return false;
    }

    uint8_t size = BLOCK_SIZE;
    status = rfid.MIFARE_Read(1, buffer, &size);

	if (status != MFRC522::STATUS_OK) {
        UART::print_error("Read failed!");
        return false;
    }

    if (size < BLOCK_SIZE)
        memset(buffer + size, 0, BLOCK_SIZE - size);

    return true;
}
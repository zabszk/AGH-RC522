#include "Card.h"
#include "Door.h"
#include "UART.h"
#include "HttpServer.h"

void setup() {
	UART::init();
	Card::init();
	Door::init();
	HttpServer::init();
}

void loop() {
	UART::tick();
	Card::tick();
	Door::tick();
	HttpServer::tick();
	sleep(0.5);
}

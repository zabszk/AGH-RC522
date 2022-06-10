#include "UART.h"

char UART::buffer[UARTBufferLength]; 
byte UART::bufferLen = 0;
bool UART::cleared = false;

void UART::init() {
	Serial.begin(9600);
}

void UART::tick() {
	if (Serial.available() <= 0)
		return;
	
	int sr = Serial.read();

	if (sr == 27) { //ESC key
		clear();
		memset(buffer, '\0', UARTBufferLength);
		bufferLen = 0;
		cleared = false;
		return;
	}
	
	if (sr == '\n' || sr == '\r' || sr == '\0') {
		if (bufferLen == 0)
			return;

		clear();
		
		Serial.printf("\033[35m>>\033[39m %s\r\n", buffer);
		process_command();
		memset(buffer, '\0', UARTBufferLength);
		bufferLen = 0;
		cleared = false;
		return;
	}

	if (sr != '\b' && (sr < 32 || sr > 126))
		return;

	if (sr == '\b') {
		if (bufferLen == 0)
			return;
			
		bufferLen--;
		buffer[bufferLen] = '\0';
		
		if (cleared)
			return;
			
		Serial.print("\b \b");
		return;
	}

	if (bufferLen >= UARTBufferLength)
		return;

	buffer[bufferLen] = (char)sr;
	bufferLen++;

	if (cleared)
		return;
	
	Serial.print((char)sr);
}

void UART::clear() {
	cleared = true;
	for (char i = 0; i < bufferLen; i++)
		Serial.print("\b \b");
}

void UART::restore() {
	Serial.printf("%s", buffer);
	cleared = false;
}

void UART::println(String s) {
	if (cleared) {
		Serial.println(s);
		return;
	}
	
	clear();
	Serial.println(s);
	restore();
}

void UART::print_error(String s) {
	println("\033[31m" + s + "\033[39m");
}

void UART::print_warning(String s) {
	println("\033[33m" + s + "\033[39m");
}

void UART::print_success(String s) {
	println("\033[32m" + s + "\033[39m");
}

void UART::process_command() {
	String command = "";
	byte args;
	
	for (args = 0; args < UARTBufferLength && buffer[args] != '\0' && buffer[args] != ' '; args++)
		command += buffer[args];

	if (args + 1 < UARTBufferLength)
		args++;

	if (command == "help") {
		print_success("--- HELP ---\r\n");
		Serial.println("- GENERAL -");
		Serial.println("help - prints this help");
		Serial.println("status (alias: s) - prints device status");
		Serial.println();
		Serial.println("- DOOR -");
		Serial.println("bistable (alias: bi) - toggles bistable mode");
		Serial.println("lockdown (alias: ld) - toggles lockdown mode");
		Serial.println("open (alias: o) - opens the door");
		Serial.println();
		Serial.println("- CARDS -");
		Serial.println("list (aliases: l, ls) - lists authorized cards");
		Serial.println("prog (alias: p) - enters/exists programming mode");
		Serial.println("wipe - removes all cards from the memory");
		return;
	}

	if (command == "bistable" || command == "bi") {
		if (Door::get_bistable()) {
			Door::set_bistable(false);
			print_success("Bistable mode deactivated!");
			return;
		}

		Door::set_bistable(true);
		print_success("Bistable mode activated!");
		return;
	}

	if (command == "list" || command == "l" || command == "ls") {
		print_success("--- LIST OF CARDS ---\r\n");
		Serial.printf(" %-3s | %-8s\r\n", "#", "UID");

		int c = 0;
		for (int i = 0; i < MAX_CARDS; i++) {
			if (Card::cards[i] == NULL)
				continue;
			
			Serial.printf(" %-3i | %08x\r\n", c, Card::cards[i]->uid);
			c++;
		}
		Serial.println("\r\n--- END OF LIST ---");
		return;
	}

	if (command == "lockdown" || command == "ld") {
		if (Door::get_lockdown()) {
			Door::set_lockdown(false);
			print_success("Lockdown deactivated!");
			return;
		}

		Door::set_lockdown(true);
		print_success("Lockdown activated!");
		return;
	}

	if (command == "open" || command == "o") {
		Door::open(true);
		print_success("Door opened!");
		return;
	}

	if (command == "prog" || command == "p") {
		if (Card::programming_mode) {
			Card::programming_mode = false;
			print_success("Exited programming mode!");
			return;
		}

		for (int i = 0; i < MAX_CARDS; i++)
			if (Card::cards[i] == NULL)
				goto enter_prog;

		UART::print_error("Cards limit reached!");
        return;

		enter_prog:
		Card::programming_mode = true;
		print_success("Entered programming mode!");
		return;
	}

	if (command == "status" || command == "s") {
		print_success("--- DEVICE STATUS ---\r\n");

		if (Door::get_bistable())
			Serial.println("Mode: BISTABLE");
		else Serial.println("Mode: MONOSTABLE");

		if (Door::get_lockdown())
			Serial.println("Lockdown: ENABLED");
		else Serial.println("Lockdown: DISABLED");

		if (Card::programming_mode)
			Serial.println("Programming mode: ENABLED");
		else Serial.println("Programming mode: DISABLED");

		return;
	}

	if (command == "wipe") {
		for (int i = 0; i < MAX_CARDS; i++) {
			if (Card::cards[i] == NULL)
				continue;

			free(Card::cards[i]->sector1);
			free(Card::cards[i]);
			Card::cards[i] = NULL;
		}
		
		print_success("All cards have been removed!");
		return;
	}

	print_error("Unknown command!");
}

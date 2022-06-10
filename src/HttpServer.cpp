#include "HttpServer.h"

const char *ssid = "RFID";
const char *password = "password1235";
WebServer HttpServer::server(80);

void HttpServer::init() {
	WiFi.softAP(ssid, password);
	server.on("/", index);
	server.on("/mono", cmd_mono);
	server.on("/bi", cmd_bi);
	server.on("/ld0", cmd_ld0);
	server.on("/ld1", cmd_ld1);
	server.on("/open", cmd_open);
	server.on("/prog", cmd_prog);
	server.on("/wipe", cmd_wipe);
	server.begin();
}

void HttpServer::index() {
	String response;

	if (Door::get_bistable())
		response = "<button onclick=\"window.location.href='/mono'\">Switch to monostable mode</button><br/>";
	else response = "<button onclick=\"window.location.href='/bi'\">Switch to bistable mode</button><br/>";

	if (Door::get_lockdown())
		response += "<button onclick=\"window.location.href='/ld0'\">Disable lockdown mode</button><br/>";
	else response += "<button onclick=\"window.location.href='/ld1'\">Enable lockdown mode</button><br/>";

	response += "<button onclick=\"window.location.href='/open'\">Open door</button><br/>";

	bool max_reached = true;

	if (!Card::programming_mode) {
		for (int i = 0; i < MAX_CARDS; i++)
			if (Card::cards[i] == NULL) {
				max_reached = false;
				break;
			}
	}
	else max_reached = false;

	if (max_reached)
		response += "<button disabled>Enter/exit programming mode</button><br/>";
	else response += "<button onclick=\"window.location.href='/prog'\">Enter/exit programming mode</button><br/>";
	response += "<button onclick=\"window.location.href='/wipe'\">Unregister all registered cards</button><br/>";

	server.send(200, "text/html", response);
}

void HttpServer::cmd_mono() {
	Door::set_bistable(false);
	index();
}

void HttpServer::cmd_bi() {
	Door::set_bistable(true);
	index();
}

void HttpServer::cmd_ld0() {
	Door::set_lockdown(false);
	index();
}

void HttpServer::cmd_ld1() {
	Door::set_lockdown(true);
	index();
}

void HttpServer::cmd_open() {
	Door::open(true);
	index();
}

void HttpServer::cmd_prog() {
	if (Card::programming_mode) {
		Card::programming_mode = false;
		index();
		return;
	}

	for (int i = 0; i < MAX_CARDS; i++)
		if (Card::cards[i] == NULL) {
			Card::programming_mode = true;
			break;
		}
	
	index();
}

void HttpServer::cmd_wipe() {
	for (int i = 0; i < MAX_CARDS; i++) {
		if (Card::cards[i] == NULL)
			continue;

		free(Card::cards[i]->sector1);
		free(Card::cards[i]);
		Card::cards[i] = NULL;
	}

	index();
}

void HttpServer::tick() {
	server.handleClient();
}

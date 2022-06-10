#pragma once

#include <WiFi.h>
#include <WebServer.h>
#include <WiFiAP.h>
#include "Card.h"
#include "Door.h"

class HttpServer {
    private:
        static WebServer server;
        static void index();
        static void cmd_mono();
        static void cmd_bi();
        static void cmd_ld0();
        static void cmd_ld1();
        static void cmd_open();
        static void cmd_prog();
        static void cmd_wipe();

    public:
        static void init();
        static void tick();
};

#pragma once

#include <ESPAsyncWebServer.h>

namespace web_server {

void init();
void start();
void stop();
AsyncWebServer* get_server();

} // namespace web_server

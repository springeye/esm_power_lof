#pragma once
#include <cstdint>
#include <cstddef>
#include <WString.h>

class AsyncWebServerRequest;

namespace ota_handler {

void handle_upload(AsyncWebServerRequest* request,
                   const String& filename,
                   size_t index,
                   uint8_t* data,
                   size_t len,
                   bool final);

int8_t get_progress();

} // namespace ota_handler

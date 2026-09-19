#ifndef REQUEST_UTILS_H
#define REQUEST_UTILS_H

#include "tinyengine/chat_protocol.h"

namespace tinyengine::utils {

std::string escape_json(std::string_view value) {
    std::string escaped;
    escaped.reserve(value.size());

    for (const char character : value) {
        switch (character) {
            case '"': escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default: escaped += character; break;
        }
    }

    return escaped;
}

std::string serialize_chat_request(const tinyengine::ChatRequest& request) {
    return "{\"model\":\"" + escape_json(request.model) +
           "\",\"messages\":[{\"role\":\"user\",\"content\":\"" +
           escape_json(request.prompt) + "\"}],\"max_tokens\":" +
           std::to_string(request.max_tokens) + ",\"temperature\":" +
           std::to_string(request.temperature) + ",\"stream\":false}";
}

size_t write_callback(char* data, size_t size, size_t nmemb, std::string* buffer) {
    if (buffer == nullptr) {
        return 0;
    }
    buffer->append(data, size * nmemb);
    return size * nmemb;
}

} // namespace tinyengine::utils

#endif // REQUEST_UTILS_H

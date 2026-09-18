#pragma once

#include <string>
#include <string_view>

namespace tinyengine {

struct ChatRequest {
    std::string model{"local-model"};
    std::string prompt;
    int max_tokens{256};
    double temperature{0.7};
};

// Serializes the small subset of OpenAI-compatible chat completion JSON used
// by the first TinyEngine client.
std::string serialize_chat_request(const ChatRequest& request);

std::string escape_json(std::string_view value);

}  // namespace tinyengine

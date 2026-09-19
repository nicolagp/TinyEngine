#pragma once

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <stdexcept>
#include <string>

namespace tinyengine {

struct ChatRequest {
    std::string model{"local-model"};
    std::string prompt;
    int max_tokens{256};
    double temperature{0.7};
};

class Client {
public:
    Client(const std::string& host, const std::string& port)
        : endpoint_("http://" + host + ":" + port + "/v1/chat/completions") {
        if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
            throw std::runtime_error("Failed to initialize libcurl");
        }
        curl_ = curl_easy_init();
        if (curl_ == nullptr) {
            curl_global_cleanup();
            throw std::runtime_error("Failed to initialize libcurl client");
        }
    }

    ~Client() {
        curl_easy_cleanup(curl_);
        curl_global_cleanup();
    }

    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;

    std::string execute(const ChatRequest& request) {
        const std::string request_body = nlohmann::json{
            {"model", request.model},
            {"messages", {{{"role", "user"}, {"content", request.prompt}}}},
            {"max_tokens", request.max_tokens},
            {"temperature", request.temperature},
            {"stream", false},
        }.dump();

        std::string response_body;
        curl_slist* headers = curl_slist_append(nullptr, "Content-Type: application/json");
        if (headers == nullptr) {
            throw std::runtime_error("Failed to create request headers");
        }

        curl_easy_reset(curl_);
        curl_easy_setopt(curl_, CURLOPT_URL, endpoint_.c_str());
        curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, request_body.c_str());
        curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE,
                         static_cast<long>(request_body.size()));
        curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, write_response);
        curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response_body);

        const CURLcode result = curl_easy_perform(curl_);
        curl_slist_free_all(headers);
        if (result != CURLE_OK) {
            throw std::runtime_error("Request failed: " +
                                     std::string(curl_easy_strerror(result)));
        }

        try {
            return nlohmann::json::parse(response_body)
                .at("choices").at(0).at("message").at("content").get<std::string>();
        } catch (const nlohmann::json::exception& error) {
            throw std::runtime_error("Unexpected response JSON: " +
                                     std::string(error.what()));
        }
    }

private:
    static size_t write_response(char* data, size_t size, size_t count,
                                 std::string* response_body) {
        response_body->append(data, size * count);
        return size * count;
    }

    std::string endpoint_;
    CURL* curl_{nullptr};
};

}  // namespace tinyengine

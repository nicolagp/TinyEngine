#include "tinyengine/chat_protocol.h"
#include "request_utils.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

#include <curl/curl.h>


namespace {

[[noreturn]] void print_usage_and_exit();

std::string require_value(int& index, int argc, char* argv[]) {
    if (++index >= argc) {
        print_usage_and_exit();
    }
    return argv[index];
}

[[noreturn]] void print_usage_and_exit() {
    std::cerr << "Usage: tinyengine_client --prompt TEXT [options]\n"
              << "  --host HOST          Server host (default: 127.0.0.1)\n"
              << "  --port PORT          Server port (default: 8080)\n"
              << "  --model MODEL        Model identifier (default: local-model)\n"
              << "  --max-tokens COUNT   Maximum generated tokens (default: 64)\n"
              << "  --temperature VALUE  Sampling temperature (default: 0.7)\n";
    std::exit(2);
}

class ClientOptions {
public:
    ClientOptions() = default;
    
    void parse_options(int argc, char* argv[]) {
        for (int index = 1; index < argc; ++index) {
            const std::string argument = argv[index];
            if (argument == "--host") {
                host = require_value(index, argc, argv);
            } else if (argument == "--port") {
                port = require_value(index, argc, argv);
            } else if (argument == "--model") {
                request.model = require_value(index, argc, argv);
            } else if (argument == "--prompt") {
                request.prompt = require_value(index, argc, argv);
            } else if (argument == "--max-tokens") {
                request.max_tokens = std::stoi(require_value(index, argc, argv));
            } else if (argument == "--temperature") {
                request.temperature = std::stod(require_value(index, argc, argv));
            } else {
                print_usage_and_exit();
            }
        }

        if (request.prompt.empty()) {
            print_usage_and_exit();
        }
    }

    std::string chat_completions_url() const {
        return "http://" + host + ":" + port + "/v1/chat/completions";
    }

    std::string request_body() const {
        return tinyengine::utils::serialize_chat_request(request);
    }

private:
    std::string host{"127.0.0.1"};
    std::string port{"8080"};
    tinyengine::ChatRequest request;
};

}  // namespace

int main(int argc, char* argv[]) {
    ClientOptions options;
    try {
        options.parse_options(argc, argv);
    } catch (const std::exception& error) {
        std::cerr << "Invalid argument: " << error.what() << "\n";
        return 2;
    }

    curl_global_init(CURL_GLOBAL_ALL);
    CURL* curl = curl_easy_init();
    if (curl == nullptr) {
        std::cerr << "Failed to initialize libcurl\n";
        curl_global_cleanup();
        return 1;
    }

    std::string response_body;
    const std::string body = options.request_body();
    const std::string url = options.chat_completions_url();
    curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, tinyengine::utils::write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);

    CURLcode result = curl_easy_perform(curl);
    if (result != CURLE_OK) {
        std::cerr << "Request failed: " << curl_easy_strerror(result) << "\n";
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return 1;
    }

    std::cout << response_body << "\n";

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    return 0;
}

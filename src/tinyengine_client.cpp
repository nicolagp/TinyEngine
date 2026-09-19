#include "tinyengine/client.h"

#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void print_usage() {
    std::cerr << "Usage: tinyengine_client [options]\n"
              << "  --host HOST          Server host (default: 127.0.0.1)\n"
              << "  --port PORT          Server port (default: 8080)\n"
              << "  --model MODEL        Model identifier (default: local-model)\n"
              << "  --max-tokens COUNT   Maximum generated tokens (default: 256)\n"
              << "  --temperature VALUE  Sampling temperature (default: 0.7)\n";
}

std::string require_value(int& index, int argc, char* argv[]) {
    if (++index >= argc) {
        throw std::invalid_argument("option requires a value");
    }
    return argv[index];
}

void parse_arguments(int argc, char* argv[], std::string& host, std::string& port,
                     tinyengine::ChatRequest& request) {
    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        if (argument == "--host") {
            host = require_value(index, argc, argv);
        } else if (argument == "--port") {
            port = require_value(index, argc, argv);
        } else if (argument == "--model") {
            request.model = require_value(index, argc, argv);
        } else if (argument == "--max-tokens") {
            request.max_tokens = std::stoi(require_value(index, argc, argv));
        } else if (argument == "--temperature") {
            request.temperature = std::stod(require_value(index, argc, argv));
        } else {
            throw std::invalid_argument("unknown option: " + argument);
        }
    }

}

}  // namespace

int main(int argc, char* argv[]) {
    std::string host{"127.0.0.1"};
    std::string port{"8080"};
    tinyengine::ChatRequest request;
    try {
        parse_arguments(argc, argv, host, port, request);
    } catch (const std::exception& error) {
        std::cerr << error.what() << "\n";
        print_usage();
        return 2;
    }

    try {
        tinyengine::Client client(host, port);
        while (true) {
            std::cout << "> " << std::flush;
            if (!std::getline(std::cin, request.prompt)) {
                std::cout << "\n";
                break;
            }
            if (request.prompt.empty()) {
                continue;
            }

            try {
                std::cout << client.execute(request) << "\n";
            } catch (const std::exception& error) {
                std::cerr << error.what() << "\n";
            }
        }
    } catch (const std::exception& error) {
        std::cerr << error.what() << "\n";
        return 1;
    }
}

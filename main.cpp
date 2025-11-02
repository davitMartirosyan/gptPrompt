#define CPPHTTPLIB_OPENSSL_SUPPORT   // <--- add this before include

#include <iostream>
#include <fstream>
#include "httplib.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

int main() {
    const char* api_key = std::getenv("OPENAI_API_KEY");
    if (!api_key) {
        std::cerr << "API key not set!" << std::endl;
        return 1;
    }
    httplib::SSLClient cli("api.openai.com");
    cli.set_default_headers({
        {"Authorization", std::string("Bearer ") + api_key},
        {"Content-Type", "application/json"}
    });

    json body = {
        {"model", "gpt-4o-mini"},
        {"messages", {
            {{"role", "user"}, {"content", "Hello, can you write a haiku about C++?"}}
        }}
    };

    auto res = cli.Post("/v1/chat/completions", body.dump(), "application/json");

    if (res && res->status == 200) {
        json response = json::parse(res->body);
        std::cout << "GPT says:\n" 
                  << response["choices"][0]["message"]["content"] << std::endl;
    } else {
        if (res)
            std::cerr << "Error: " << res->status << "\n" << res->body << std::endl;
        else
            std::cerr << "Request failed. Check your network or SSL." << std::endl;
    }

    return 0;
}

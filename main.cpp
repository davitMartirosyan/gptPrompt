#define CPPHTTPLIB_OPENSSL_SUPPORT // <-- CRITICAL: Must be defined before including httplib.h
#include <iostream>
#include <string>
#include <cstdlib>
#include "json.hpp"
#include "httplib.h" // Assuming you have this header in your path

using json = nlohmann::json;

// --- Function Prototypes ---
std::string get_api_key();
void handle_gemini_request(const std::string& prompt);

/**
 * @brief Simple console application to send a prompt to the Gemini API.
 * This application constructs the required JSON payload, sends it via a POST
 * request using cpp-httplib, and parses the response using nlohmann::json.
 */
int main() {
    // 1. Check for API Key
    if (get_api_key().empty()) {
        std::cerr << "ERROR: GEMINI_API_KEY environment variable not set." << std::endl;
        std::cerr << "Please set it before running the application." << std::endl;
        return 1;
    }

    std::cout << "--- C++ Gemini Console Client (gemini-2.5-flash) ---" << std::endl;
    std::cout << "Enter your prompt (type 'quit' to exit):" << std::endl;

    std::string prompt;
    // Loop to read user input
    while (std::getline(std::cin, prompt)) {
        if (prompt == "quit") {
            break;
        }
        if (prompt.empty()) {
            continue;
        }

        std::cout << "\n[Querying Gemini...]" << std::endl;
        handle_gemini_request(prompt);
        std::cout << "\nEnter your next prompt (type 'quit' to exit):" << std::endl;
    }

    std::cout << "Application closed." << std::endl;
    return 0;
}

/**
 * @brief Retrieves the Gemini API key from the environment variable.
 * @return The API key string.
 */
std::string get_api_key() {
    const char* api_key_env = std::getenv("GEMINI_API_KEY");
    if (api_key_env) {
        return api_key_env;
    }
    return "";
}

/**
 * @brief Constructs and sends the HTTP request to the Gemini API.
 * @param prompt The user's text prompt.
 */
void handle_gemini_request(const std::string& prompt) {
    // --- API Configuration ---
    // Host without the "https://" prefix for SSLClient
    const std::string API_HOST = "generativelanguage.googleapis.com";
    // We use the generateContent endpoint with a capable, high-speed model
    const std::string API_PATH = "/v1beta/models/gemini-2.5-flash:generateContent";
    const std::string API_KEY = get_api_key();

    // 1. Build the Request Body using nlohmann::json
    json request_body = {
        {"contents", {
            {{"parts", {
                {{"text", prompt}}
            }}}
        }},
        {"generationConfig", { // <-- CORRECTED: Must be "generationConfig"
            // Optional: Adjust temperature for more creative/deterministic output
            {"temperature", 0.7} 
        }}
    };

    // Convert the JSON object to a string for the request body
    std::string body_string = request_body.dump(4); // `4` for indentation

    // 2. Initialize the Client
    // USE httplib::SSLClient for HTTPS connections
    httplib::SSLClient cli(API_HOST);

    // Optional: Enable server certificate verification for production use
    // cli.enable_server_certificate_verification(true); 

    // 3. Set Headers
    // The API key is passed in the "x-goog-api-key" header
    httplib::Headers headers = {
        {"Content-Type", "application/json"},
        {"x-goog-api-key", API_KEY}
    };

    // 4. Send the POST request
    auto res = cli.Post(API_PATH.c_str(), headers, body_string, "application/json");

    // 5. Handle the Response
    if (res && res->status == 200) {
        // Successful API call
        try {
            json response_json = json::parse(res->body);

            // Navigate the JSON path to the generated text
            // response_json["candidates"][0]["content"]["parts"][0]["text"]
            std::string generated_text = response_json
                .at("candidates").at(0)
                .at("content").at("parts").at(0)
                .at("text")
                .get<std::string>();

            std::cout << "\n[Gemini Response]:" << std::endl;
            std::cout << "--------------------------------------" << std::endl;
            std::cout << generated_text << std::endl;
            std::cout << "--------------------------------------" << std::endl;

        } catch (const json::exception& e) {
            std::cerr << "\nJSON PARSE ERROR: Failed to parse response or extract content." << std::endl;
            std::cerr << "Error details: " << e.what() << std::endl;
            // Print the raw body if parsing failed
            std::cerr << "Raw response body: " << res->body << std::endl;
        }

    } else {
        // Handle HTTP errors or non-200 status codes
        std::cerr << "\nHTTP REQUEST FAILED." << std::endl;
        if (res) {
            std::cerr << "Status Code: " << res->status << std::endl;
            std::cerr << "Response Body (if available): " << res->body << std::endl;
        } else {
            // Transport error. 
            // The previous error `cli.get_last_error()` was replaced in the previous turn.
            // We cast to int to print the raw error code from the httplib::Error enum.
            std::cerr << "Client Transport Error Code: " << static_cast<int>(res.error()) << std::endl;
        }
    }
}

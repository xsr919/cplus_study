#include "HttpServer.h"
#include <iostream>
#include <signal.h>
#include <sstream>

HttpServer* g_server = nullptr;

void signalHandler(int signal) {
    if (signal == SIGINT && g_server) {
        std::cout << "\nShutting down server..." << std::endl;
        g_server->stop();
    }
}

int main(int argc, char* argv[]) {
    int port = 8080;
    
    if (argc > 1) {
        port = std::atoi(argv[1]);
    }
    
    try {
        HttpServer server(port);
        g_server = &server;
        
        signal(SIGINT, signalHandler);
        
        server.route("/", [](const std::string& request) {
            return R"(
<!DOCTYPE html>
<html>
<head>
    <title>Simple HTTP Server</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; }
        h1 { color: #333; }
        .info { background-color: #f0f0f0; padding: 20px; border-radius: 5px; }
    </style>
</head>
<body>
    <h1>Welcome to Simple HTTP Server</h1>
    <div class="info">
        <p>This is a simple HTTP server built with C++</p>
        <p>Available endpoints:</p>
        <ul>
            <li><a href="/">/</a> - Home page</li>
            <li><a href="/about">/about</a> - About page</li>
            <li><a href="/hello">/hello</a> - Hello page</li>
            <li><a href="/time">/time</a> - Current server time</li>
        </ul>
    </div>
</body>
</html>)";
        });
        
        server.route("/about", [](const std::string& request) {
            return R"(
<!DOCTYPE html>
<html>
<head>
    <title>About</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; }
        h1 { color: #333; }
    </style>
</head>
<body>
    <h1>About This Server</h1>
    <p>This is a simple HTTP server implementation in C++</p>
    <p>Features:</p>
    <ul>
        <li>Multi-threaded request handling</li>
        <li>Basic routing support</li>
        <li>HTTP/1.1 protocol</li>
    </ul>
    <p><a href="/">Back to Home</a></p>
</body>
</html>)";
        });
        
        server.route("/hello", [](const std::string& request) {
            return R"(
<!DOCTYPE html>
<html>
<head>
    <title>Hello</title>
    <style>
        body { 
            font-family: Arial, sans-serif; 
            margin: 40px;
            text-align: center;
        }
        h1 { 
            color: #4CAF50; 
            font-size: 48px;
        }
    </style>
</head>
<body>
    <h1>Hello, World!</h1>
    <p>Greetings from C++ HTTP Server</p>
    <p><a href="/">Back to Home</a></p>
</body>
</html>)";
        });
        
        server.route("/time", [](const std::string& request) {
            time_t now = time(0);
            char* time_str = ctime(&now);
            
            std::ostringstream html;
            html << R"(
<!DOCTYPE html>
<html>
<head>
    <title>Server Time</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; text-align: center; }
        .time { font-size: 24px; color: #2196F3; margin: 20px; }
    </style>
</head>
<body>
    <h1>Current Server Time</h1>
    <div class="time">)" << time_str << R"(</div>
    <p><a href="/">Back to Home</a></p>
</body>
</html>)";
            return html.str();
        });
        
        std::cout << "Starting HTTP server on port " << port << "..." << std::endl;
        std::cout << "Open your browser and navigate to http://localhost:" << port << std::endl;
        std::cout << "Press Ctrl+C to stop the server" << std::endl;
        
        server.start();
        
        while (server.isRunning()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
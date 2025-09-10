#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <string>
#include <map>
#include <functional>
#include <thread>
#include <atomic>

class HttpServer {
public:
    using RequestHandler = std::function<std::string(const std::string& request)>;

    struct HttpRequest {
        std::string method;
        std::string path;
        std::string version;
        std::map<std::string, std::string> headers;
        std::string body;
    };

    struct HttpResponse {
        int status_code = 200;
        std::string status_text = "OK";
        std::map<std::string, std::string> headers;
        std::string body;
        std::string toString() const;
    };

    HttpServer(int port = 8080);
    ~HttpServer();

    void route(const std::string& path, RequestHandler handler);
    void start();
    void stop();
    bool isRunning() const;

private:
    int port_;
    int server_fd_;
    std::atomic<bool> running_;
    std::map<std::string, RequestHandler> routes_;
    std::thread server_thread_;

    void acceptConnections();
    void handleClient(int client_socket);
    HttpRequest parseRequest(const std::string& raw_request);
    HttpResponse processRequest(const HttpRequest& request);
    std::string readFullRequest(int client_socket);
};

#endif
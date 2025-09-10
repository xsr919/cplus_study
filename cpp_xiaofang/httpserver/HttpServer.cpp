#include "HttpServer.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

std::string HttpServer::HttpResponse::toString() const {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << status_code << " " << status_text << "\r\n";

    for (const auto& [key, value] : headers) {
        oss << key << ": " << value << "\r\n";
    }
    if (!body.empty() && headers.find("Content-Length") == headers.end()) {
        oss << "Content-Length: " << body.length() << "\r\n" ;
    }

    oss << "\r\n";
    oss << body;
    return oss.str();
}

HttpServer::HttpServer(int port) : port_(port), server_fd_(-1), running_(false) {}

HttpServer::~HttpServer() {
    stop();
}

void HttpServer::route(const std::string& path, RequestHandler handler) {
    routes_[path] = handler;
}

void HttpServer::start() {
    if (running_) {
        return;
    }

    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        throw std::runtime_error("Failed to create socket");
    }

    int opt = 1;
    if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(server_fd_);
        throw std::runtime_error("Failed to set socket options");
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);

    if (bind(server_fd_, (struct sockaddr*)&address, sizeof(address)) < 0) {
        close(server_fd_);
        throw std::runtime_error("Failed to bind socket");
    }

    if (listen(server_fd_, 10) < 0) {
        close(server_fd_);
        throw std::runtime_error("Failed to listen on socket");
    }

    running_ = true;
    server_thread_ = std::thread(&HttpServer::acceptConnections, this);

    std::cout << "Server started on port" << port_ << std::endl;
}

void HttpServer::stop() {
    if (!running_) {
        return ;
    }

    running_ = false;
    if (server_fd_ >= 0) {
        shutdown(server_fd_, SHUT_RDWR);
        close(server_fd_);
    }

    if (server_thread_.joinable()) {
        server_thread_.join();
    }
    std::cout << "Server stopped" << std::endl;
}

bool HttpServer::isRunning() const {
    return running_;
}

void HttpServer::acceptConnections() {
    while (running_) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_socket = accept(server_fd_, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            if (running_) {
                std::cerr << "Failed to accept connection" << std::endl;
            }
            continue;
        }

        std::thread client_thread(&HttpServer::handleClient, this, client_socket);
        client_thread.detach();
    }
}


void HttpServer::handleClient(int client_socket) {
    try {
        std::string raw_request = readFullRequest(client_socket);

        if (!raw_request.empty()) {
            HttpRequest request = parseRequest(raw_request);
            HttpResponse response = processRequest(request);

            std::string response_str = response.toString();
            send(client_socket, response_str.c_str(), response_str.length(), 0);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error handling client: " << e.what() << std::endl;
    }

    close(client_socket);
}

std::string HttpServer::readFullRequest(int client_socket) {
    char buffer[4096];
    std::string request;
    ssize_t bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        request = buffer;
    }

    return request;
}

HttpServer::HttpRequest HttpServer::parseRequest(const std::string& raw_request) {
    HttpRequest request;
    std::istringstream iss(raw_request);
    std::string line;

    if (std::getline(iss, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        std::istringstream line_stream(line);
        line_stream >> request.method >> request.path >> request.version;
    }

    while (std::getline(iss, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (line.empty()) {
            break;
        }

        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string key = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);
            size_t start = value.find_first_not_of(" \t");
            if (start != std::string::npos) {
                value = value.substr(start);
            }
            request.headers[key] = value;
        }
    }

    std::string remaining;
    while (std::getline(iss, line)) {
        remaining += line + "\n";
    }
    if (!remaining.empty() && remaining.back() == '\n') {
        remaining.pop_back();
    }
    request.body = remaining;

    return request;
}

HttpServer::HttpResponse HttpServer::processRequest(const HttpRequest& request) {
    HttpResponse response;

    std::cout << "Request: " << request.method << " " << request.path << std::endl;

    auto it = routes_.find(request.path);
    if (it != routes_.end()) {
        std::ostringstream request_str;
        request_str << request.method << " " << request.path << " " << request.version << "\r\n";
        for (const auto& [key, value] : request.headers) {
            request_str << key << ": " << value << "\r\n";
        }
        request_str << "\r\n" << request.body;

        response.body = it->second(request_str.str());
        response.headers["Content-Type"] = "text/html";
    } else {
        response.status_code = 404;
        response.status_text = "Not Found";
        response.body = "<html><body><h1>404 Not Found</h1></body></html>";
        response.headers["Content-Type"] = "text/html";
    }

    response.headers["Server"] = "SimpleHttpServer/1.0";
    response.headers["Connection"] = "close";

    return response;
}
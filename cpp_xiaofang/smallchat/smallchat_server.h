#pragma once
#include <memory>
#include <string>
#include <vector>

extern "C" {
    #include "chatlib.h"
}

class Client {
public:
    explicit Client(int fd);
    ~Client();

    int fd() const;
    const std::string& nick() const;
    void setNick(std::string s);
    bool send(const std::string& s) const;

private:
    int fd_{-1};
    std::string nick_;
};

class ChatServer {
public:
    ChatServer();
    ~ChatServer();

    void run();
private:
    void acceptNewClient();
    void removeClient(int fd);
    void handleIncoming(Client& c, char* raw);
    void broadcastExcept(int excluded_fd, const std::string& s);
    static void trimCrlf(char* s);

    int server_fd_{-1};
    int max_client_fd_{-1};
    std::vector<std::unique_ptr<Client>> clients;
};



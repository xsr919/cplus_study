#include "smallchat_server.h"
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>

// =============== Client 实现 =================

Client::Client(int fd) : fd_(fd), nick_("user:" + std::to_string(fd)) {
    socketSetNonBlockNoDelay(fd_);
}

Client::~Client() {
    if(fd_ >= 0) ::close(fd_);
}

int Client::fd() const 
{
    return fd_;
}

const std::string& Client::nick() const 
{
    return nick_;
}

void Client::setNick(std::string s)
{
    nick_ = std::move(s);
}

bool Client::send(const std::string& s) const
{
    if(fd_ < 0) return false;
    ssize_t n = ::write(fd_, s.data(), s.size());
    return n == static_cast<ssize_t>(s.size());
}

// =============== ChatServer 实现 ================

ChatServer::ChatServer() 
{
    server_fd_ = createTCPServer(7711);
    if(server_fd_ == -1) {
        perror("Creating listening socket");
        std::exit(1);
    }
    clients.resize(FD_SETSIZE);
    max_client_fd_ = -1;
}

ChatServer::~ChatServer() 
{
    clients.clear();
    if(server_fd_ >= 0) ::close(server_fd_);
}

void ChatServer::run() 
{
    for(;;) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(server_fd_, &readfds);

        for(int fd = 0; fd <= max_client_fd_; ++fd) {
            if(clients[fd]) FD_SET(fd, &readfds);
        }

        timeval tv{1, 0};
        int maxfd = std::max(server_fd_, max_client_fd_);
        int ready = ::select(maxfd + 1, &readfds, nullptr, nullptr, &tv);
        if(ready < 0) {
            perror("select() error");
            std::exit(1);
        }

        if(ready == 0) continue;

        if(FD_ISSET(server_fd_, &readfds)) {
            acceptNewClient();
        }

        char buf[256];
        for(int fd = 0; fd <= max_client_fd_; ++fd) {
            if(!clients[fd]) continue;
            if(!FD_ISSET(fd, &readfds)) continue;

            int nread = ::read(fd, buf, sizeof(buf) - 1);
            if(nread <= 0) {
                printf("Disconnected client fd=%d, nick=%s\n", fd, clients[fd]->nick().c_str());
                removeClient(fd);
                continue;
            }

            buf[nread] = '\0';
            handleIncoming(*clients[fd], buf);
        }
    }
}

void ChatServer::acceptNewClient()
{
    int cfd = acceptClient(server_fd_);
    if(cfd < 0 || cfd >= FD_SETSIZE){
        if(cfd >= 0) ::close(cfd);
        return ;
    }

    if(clients[cfd]) {
        ::close(cfd);
        return;
    }
    clients[cfd] = std::make_unique<Client>(cfd);
    max_client_fd_ = std::max(max_client_fd_, cfd);

    static const char kWelcome[] = "Welcome to simple chat! Use /nick to set your nick.\n";
    clients[cfd]->send(kWelcome);
    printf("Connected client fd=%d\n", cfd);
}

void ChatServer::removeClient(int fd) 
{
    if(fd < 0 || fd >= (int)clients.size() || !clients[fd]) return ;
    clients[fd].reset();
    if(fd == max_client_fd_) {
        for(int j = max_client_fd_ - 1; j >= 0; --j) {
            if(clients[j]) {
                max_client_fd_ = j;
                return ;
            }
        }
        max_client_fd_ = -1;
    }
}

void ChatServer::trimCrlf(char* s) 
{
    if(!s) return;
    for(char* p = s; *p; ++p) {
        if(*p == '\r' || *p == '\n') {
            *p = '\0';
            break;
        }
    }
}

void ChatServer::handleIncoming(Client& c, char* raw)
{
    if(raw[0] == '/') {
        trimCrlf(raw);
        char* space = std::strchr(raw, ' ');
        const char* cmd = raw;
        const char* arg = nullptr;
        if(space) {
            *space = '\0';
            arg = space + 1;
        }
        if(std::strcmp(cmd, "/nick") == 0 && arg && *arg) {
            c.setNick(std::string(arg));
        } else {
            static const char kErr[] = "Unsupported command\n";
            c.send(kErr);
        }
        return ;
    }
    std::string line(raw);
    std::string msg = c.nick() + "> " + line;
    std::fwrite(msg.data(), 1, msg.size(), stdout);
    broadcastExcept(c.fd(), msg);
}

void ChatServer::broadcastExcept(int excluded_fd, const std::string& s) 
{
    for(int fd = 0; fd <= max_client_fd_; ++fd) {
        if(!clients[fd]) continue;
        if(fd == excluded_fd) continue;
        clients[fd]->send(s);
    }
}

int main()
{
    ChatServer server;
    server.run();
    return 0;
}
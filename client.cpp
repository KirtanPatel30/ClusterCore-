#include "client.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#include <iostream>
#include <sstream>

Client::Client(std::string host, int port)
    : host_(std::move(host)), port_(port), connect_fd_(-1) {}

int Client::connect_once() {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    addr.sin_addr.s_addr = inet_addr(host_.c_str());
    if (::connect(fd, (sockaddr*)&addr, sizeof(addr)) != 0) {
        ::close(fd);
        return -1;
    }
    return fd;
}

std::string Client::send_line(const std::string& line) {
    int fd = connect_once();
    if (fd < 0) return "ERR_CONNECT\n";
    std::string s = line + "\n";
    const char* p = s.c_str();
    size_t left = s.size();
    while (left > 0) {
        ssize_t n = ::send(fd, p, left, 0);
        if (n <= 0) { ::close(fd); return "ERR_SEND\n"; }
        p += n; left -= n;
    }
    // read response
    char buf[4096];
    std::string acc;
    while (true) {
        ssize_t n = ::recv(fd, buf, sizeof(buf), 0);
        if (n <= 0) break;
        acc.append(buf, buf + n);
        if (acc.find('\n') != std::string::npos) break;
    }
    ::close(fd);
    return acc;
}

void Client::repl() {
    std::cout << "[client] connected to " << host_ << ":" << port_ << "\n";
    std::cout << "Commands: PUT <k> <v> | GET <k> | DELETE <k> | PEERS | EXIT\n";
    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;
        if (line == "EXIT" || line == "quit") break;
        auto resp = send_line(line);
        std::cout << resp;
    }
}

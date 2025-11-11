#pragma once
#include <string>

class Client {
public:
    Client(std::string host, int port);
    void repl();

private:
    std::string host_;
    int port_;
    int connect_fd_;
    int connect_once();
    std::string send_line(const std::string& line);
};

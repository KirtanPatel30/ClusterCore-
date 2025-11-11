#include <iostream>
#include <string>
#include "client.hpp"

int main(int argc, char** argv) {
    std::string host = "127.0.0.1";
    int port = 5000;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--host" && i+1 < argc) host = argv[++i];
        else if (a == "--port" && i+1 < argc) port = std::stoi(argv[++i]);
        else if (a == "--help") {
            std::cout << "Usage: " << argv[0] << " --host 127.0.0.1 --port 5000\n";
            return 0;
        }
    }
    Client c(host, port);
    c.repl();
    return 0;
}

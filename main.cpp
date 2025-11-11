#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include "server.hpp"

static std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> parts;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        if (!item.empty()) parts.push_back(item);
    }
    return parts;
}

int main(int argc, char** argv) {
    std::string host = "127.0.0.1";
    int port = 5000;
    std::string peers_arg;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--host" && i+1 < argc) host = argv[++i];
        else if (a == "--port" && i+1 < argc) port = std::stoi(argv[++i]);
        else if (a == "--peers" && i+1 < argc) peers_arg = argv[++i];
        else if (a == "--help") {
            std::cout << "Usage: " << argv[0] << " --host 127.0.0.1 --port 5000 --peers 127.0.0.1:5001,127.0.0.1:5002\n";
            return 0;
        }
    }

    std::vector<Peer> peers;
    // Self first
    peers.push_back(Peer{host, port});
    // Parse peers and append
    if (!peers_arg.empty()) {
        auto entries = split(peers_arg, ',');
        for (auto &e : entries) {
            auto hp = split(e, ':');
            if (hp.size() == 2) {
                peers.push_back(Peer{hp[0], std::stoi(hp[1])});
            }
        }
    }

    Server server(host, port, peers);
    server.start();

    // Block main thread
    std::cout << "[node] running at " << host << ":" << port << " with " << peers.size() << " nodes\n";
    // Keep alive
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(60));
    }
    return 0;
}

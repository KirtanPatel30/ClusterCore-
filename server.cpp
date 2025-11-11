#include "server.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <sstream>

static std::vector<std::string> split_ws(const std::string& s) {
    std::istringstream iss(s);
    std::vector<std::string> v;
    std::string w;
    while (iss >> w) v.push_back(w);
    return v;
}

Server::Server(std::string host, int port, std::vector<Peer> peers)
    : host_(std::move(host)), port_(port), listen_fd_(-1), running_(false), peers_(std::move(peers))
{
    // Build hash ring list of addresses (strings)
    std::vector<std::string> nodes;
    nodes.reserve(peers_.size());
    for (auto &p : peers_) nodes.push_back(p.str());
    ring_.set_nodes(nodes);

    // find self index
    self_index_ = 0;
    std::string addr = address();
    for (size_t i = 0; i < nodes.size(); ++i) {
        if (nodes[i] == addr) { self_index_ = i; break; }
    }
}

Server::~Server() { stop(); }

void Server::start() {
    listen_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd_ < 0) { perror("socket"); exit(1); }

    int opt = 1;
    setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    addr.sin_addr.s_addr = inet_addr(host_.c_str());

    if (::bind(listen_fd_, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind"); exit(1);
    }
    if (::listen(listen_fd_, 128) < 0) {
        perror("listen"); exit(1);
    }

    running_.store(true);
    std::thread(&Server::accept_loop, this).detach();
}

void Server::stop() {
    running_.store(false);
    if (listen_fd_ >= 0) close(listen_fd_);
    listen_fd_ = -1;
}

void Server::accept_loop() {
    std::cout << "[node] listening on " << host_ << ":" << port_ << std::endl;
    while (running_.load()) {
        sockaddr_in caddr{};
        socklen_t clen = sizeof(caddr);
        int cfd = ::accept(listen_fd_, (sockaddr*)&caddr, &clen);
        if (cfd < 0) {
            if (errno == EINTR) continue;
            perror("accept");
            continue;
        }
        std::thread(&Server::handle_client, this, cfd).detach();
    }
}

void write_all(int fd, const std::string& s) {
    const char* p = s.c_str();
    size_t left = s.size();
    while (left > 0) {
        ssize_t n = ::send(fd, p, left, 0);
        if (n <= 0) return;
        p += n;
        left -= n;
    }
}

std::string Server::process_command(const std::string& line) {
    auto toks = split_ws(line);
    if (toks.empty()) return "ERR\n";
    auto cmd = toks[0];

    if (cmd == "PUT" && toks.size() >= 3) {
        std::string key = toks[1];
        std::string value = line.substr(line.find(key) + key.size() + 1);
        // value might contain spaces; remove leading cmd+key
        size_t pos_val = value.find(' ');
        if (pos_val != std::string::npos) value = value.substr(pos_val + 1);

        // route to primary
        if (!is_primary_for(key)) {
            return "REDIRECT\n";
        }
        store_.put(key, value);
        replicate_put(key, value);
        return "OK\n";
    }
    if (cmd == "GET" && toks.size() == 2) {
        std::string key = toks[1];
        if (!is_primary_for(key)) return "REDIRECT\n";
        auto v = store_.get(key);
        if (v.has_value()) return v.value() + "\n";
        return "NOT_FOUND\n";
    }
    if (cmd == "DELETE" && toks.size() == 2) {
        std::string key = toks[1];
        if (!is_primary_for(key)) return "REDIRECT\n";
        bool ok = store_.remove(key);
        return ok ? "OK\n" : "NOT_FOUND\n";
    }
    if (cmd == "PEERS") {
        std::ostringstream oss;
        for (size_t i = 0; i < peers_.size(); ++i) {
            oss << peers_[i].str() << (i+1<peers_.size() ? "," : "");
        }
        oss << "\n";
        return oss.str();
    }
    // Replication command (no re-broadcast)
    if (cmd == "REPL_PUT" && toks.size() >= 3) {
        std::string key = toks[1];
        std::string value = line.substr(line.find(key) + key.size() + 1);
        size_t pos_val = value.find(' ');
        if (pos_val != std::string::npos) value = value.substr(pos_val + 1);
        store_.put(key, value);
        return "OK\n";
    }
    return "ERR\n";
}

void Server::handle_client(int client_fd) {
    char buf[4096];
    std::string acc;
    while (true) {
        ssize_t n = ::recv(client_fd, buf, sizeof(buf), 0);
        if (n <= 0) break;
        acc.append(buf, buf + n);
        size_t pos;
        while ((pos = acc.find('\n')) != std::string::npos) {
            std::string line = acc.substr(0, pos);
            acc.erase(0, pos + 1);
            auto resp = process_command(line);
            write_all(client_fd, resp);
        }
    }
    ::close(client_fd);
}

bool Server::is_primary_for(const std::string& key) const {
    if (ring_.size() == 0) return true;
    size_t idx = ring_.pick_index(key);
    return idx == self_index_;
}

// naive replication: fire-and-forget to all other nodes
void Server::replicate_put(const std::string& key, const std::string& value) {
    for (size_t i = 0; i < peers_.size(); ++i) {
        if (i == self_index_) continue;
        int fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) continue;

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(peers_[i].port);
        addr.sin_addr.s_addr = inet_addr(peers_[i].host.c_str());

        if (::connect(fd, (sockaddr*)&addr, sizeof(addr)) == 0) {
            std::string line = "REPL_PUT " + key + " " + value + "\n";
            write_all(fd, line);
        }
        ::close(fd);
    }
}

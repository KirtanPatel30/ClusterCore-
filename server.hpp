#pragma once
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <netinet/in.h>
#include "kv_store.hpp"
#include "hash_ring.hpp"

struct Peer {
    std::string host;
    int port;
    std::string str() const { return host + ":" + std::to_string(port); }
};

class Server {
public:
    Server(std::string host, int port, std::vector<Peer> peers);
    ~Server();
    void start();
    void stop();

    // For replication
    void replicate_put(const std::string& key, const std::string& value);
    bool is_primary_for(const std::string& key) const;

private:
    std::string host_;
    int port_;
    int listen_fd_;
    std::atomic<bool> running_;
    KVStore store_;
    std::vector<Peer> peers_;
    HashRing ring_;
    size_t self_index_;

    void accept_loop();
    void handle_client(int client_fd);
    std::string process_command(const std::string& line);
    std::string address() const { return host_ + ":" + std::to_string(port_); }
};

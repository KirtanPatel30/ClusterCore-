#pragma once
#include <vector>
#include <string>
#include <functional>

// Simple hash ring: not true consistent hashing with virtual nodes,
// but stable indexing by hashing the key and modding by N.
class HashRing {
public:
    HashRing() = default;
    explicit HashRing(const std::vector<std::string>& nodes)
        : nodes_(nodes) {}

    void set_nodes(const std::vector<std::string>& nodes) { nodes_ = nodes; }
    size_t pick_index(const std::string& key) const {
        if (nodes_.empty()) return 0;
        std::hash<std::string> h;
        return h(key) % nodes_.size();
    }
    size_t size() const { return nodes_.size(); }

private:
    std::vector<std::string> nodes_;
};

#pragma once
#include <unordered_map>
#include <string>
#include <mutex>
#include <optional>

class KVStore {
public:
    std::optional<std::string> get(const std::string &key);
    void put(const std::string &key, const std::string &value);
    bool remove(const std::string &key);

private:
    std::unordered_map<std::string, std::string> store_;
    std::mutex mtx_;
};

#include "kv_store.hpp"

std::optional<std::string> KVStore::get(const std::string &key) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = store_.find(key);
    if (it == store_.end()) return std::nullopt;
    return it->second;
}

void KVStore::put(const std::string &key, const std::string &value) {
    std::lock_guard<std::mutex> lock(mtx_);
    store_[key] = value;
}

bool KVStore::remove(const std::string &key) {
    std::lock_guard<std::mutex> lock(mtx_);
    return store_.erase(key) > 0;
}

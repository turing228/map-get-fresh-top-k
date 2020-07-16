// AccurateFrequencyAnalyzer implementation -*- C++ -*-

// Made especially for VK Team by Nikita Lisovetin (vk: vk.com/nikitalisovetin, e-mail: nik-lisovetin@ya.ru)

#ifndef VKTEST_ACCURATE_FREQUENCY_ANALYZER_H
#define VKTEST_ACCURATE_FREQUENCY_ANALYZER_H

#include <math.h>

#include <set>
#include <ctime>
#include <chrono>
#include <cstdlib>
#include <random>
#include <string>
#include <algorithm>
#include <map>

/**
 *  @brief Absolutely exact analyzer for getting keys asked >=10% for the last minute by default.
 *
 *  @tparam Key  Type of key objects, defaults to std::string.
 *
 *  It's made only for testing purposes to check the correctness of map_with_get_very_frequent class. It eats a lot of
 *  memory and cannot be easily used for highload tasks. Logs each new key (save timestamp and key). When asked,
 *  analyzes logs and return keys asked >=10% for the last minute by default.
 */
template<typename Key = std::string>
class AccurateFrequencyAnalyzer {
public:
    typedef Key key_type;

    explicit AccurateFrequencyAnalyzer(std::chrono::duration<double> control_time = std::chrono::microseconds(1000000));

    void add(key_type new_key);

    std::vector<key_type> GetActualTop(std::chrono::system_clock::time_point time = std::chrono::system_clock::now());

private:
    struct KeyInfo {
    public:
        explicit KeyInfo(key_type key = key_type(),
                         std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now());

        bool operator<(const KeyInfo &b);

        std::chrono::time_point<std::chrono::system_clock> created_at;

        key_type key;
    };

    void DeleteOldLogs(std::chrono::system_clock::time_point time = std::chrono::system_clock::now());

    const std::chrono::duration<double> control_time_;

    int64_t new_elements_since_delete_old_logs_ = 0;
    std::vector<KeyInfo> data_;

};

template<typename Key>
AccurateFrequencyAnalyzer<Key>::KeyInfo::KeyInfo(key_type key, std::chrono::time_point<std::chrono::system_clock> time)
        : key(key), created_at(time) {

}

template<typename Key>
bool AccurateFrequencyAnalyzer<Key>::KeyInfo::operator<(const AccurateFrequencyAnalyzer::KeyInfo &b) {
    return created_at < b.created_at;
}

template<typename Key>
AccurateFrequencyAnalyzer<Key>::AccurateFrequencyAnalyzer(std::chrono::duration<double> control_time)
        : control_time_(control_time) {

}

template<typename Key>
void AccurateFrequencyAnalyzer<Key>::add(key_type new_key) {
    new_elements_since_delete_old_logs_++;
    if (new_elements_since_delete_old_logs_ % 1000000 == 0) {
        DeleteOldLogs();
    }
    data_.push_back(KeyInfo(new_key));
}

template<typename Key>
std::vector<typename AccurateFrequencyAnalyzer<Key>::key_type>
AccurateFrequencyAnalyzer<Key>::GetActualTop(std::chrono::time_point<std::chrono::system_clock> time) {

    std::chrono::system_clock::time_point time_since = std::chrono::time_point_cast<std::chrono::microseconds>(
            time - control_time_);
    DeleteOldLogs(time_since);

    int64_t num = data_.size();
    std::vector<key_type> result;
    if (num >= 100) {
        std::map<key_type, int64_t> counter;
        for (auto it = data_.begin(); it != data_.end(); ++it) {
            auto it_element = counter.find(it->key);
            if (it_element != counter.end()) {
                ++it_element->second;
            } else {
                counter[it->key] = 1;
            }
        }

        for (auto it = counter.begin(); it != counter.end(); ++it) {
            if (it->second > num * 0.1) {
                result.push_back(it->first);
            }
        }
    }

    return result;
}

template<typename Key>
void AccurateFrequencyAnalyzer<Key>::DeleteOldLogs(std::chrono::time_point<std::chrono::system_clock> time_since) {
    new_elements_since_delete_old_logs_ = 0;
    auto lower = std::lower_bound(data_.begin(), data_.end(), KeyInfo(key_type(), time_since));
    data_.erase(data_.begin(), lower);
}

#endif //VKTEST_ACCURATE_FREQUENCY_ANALYZER_H

// FrequencyEstimationAnalyzer implementation -*- C++ -*-

// Made especially for VK Team by Nikita Lisovetin (vk: vk.com/nikitalisovetin, e-mail: nik-lisovetin@ya.ru)

#ifndef VKTEST_FREQUENCY_ESTIMATION_ANALYZER_H
#define VKTEST_FREQUENCY_ESTIMATION_ANALYZER_H

#include <string>
#include <map>
#include <chrono>
#include <cmath>
#include <list>
#include <vector>
#include <utility>
#include <algorithm>
#include <iostream>
#include <exception>

/**
 *  @brief Duplicate key request frequency analyzer.
 *
 *  @tparam Key  Type of key objects, defaults to std::string
 *  @tparam Compare  Comparison function object type, defaults to less<Key>.
 *  @tparam Alloc  Allocator type, defaults to allocator<const Key>.
 *
 *  Analyzer supports actual statistics for the last `control_time` time. It allows implementing the "show very
 *  frequently asked keys" function. Inside of it is a lot of buckets (small analyzers) - temporary objects what are
 *  keeping statistics for all the time since creation time. The statistics from the oldest bucket is considered as
 *  "actual". Look README.md for more details.
 */
template<typename Key = std::string, typename Compare = std::less<Key>>
class FrequencyEstimationAnalyzer {
public:
    /**
     *  @brief Duplicate key request frequency analyzer constructor.
     *
     *  @param control_time  Timespan, defaults to 60 seconds.
     *  @param share_very_frequent  Share of requests for keys to be considered as "very frequent", defaults to 0.1
     *  (10%).
     *  @param num_buckets  Amount of buckets, defaults to 12.
     *  @param bucket_size  Size of each bucket, defaults to 54.
     */
    explicit FrequencyEstimationAnalyzer(std::chrono::duration<double> control_time = std::chrono::seconds(1),
                                         double share_very_frequent = 0.1, size_t num_buckets = 12,
                                         size_t bucket_size = 54);

    /**
     *  @brief  Transfer information about a newly added key.
     *  @param  key  Added a key.
     *
     *  Time complexity: O(1).
     */
    void AddKey(const Key &key);

    /**
     *  @brief  Get vector of very frequently asked keys (>= ~10%) for the last time
     *
     *  @param number  Number of the requested top by frequency of requests in the last period keys.
     *  @return  Vector of very frequently asked keys for the last time.
     *
     *  Vector size can be different. If `number` is specified, returns min of requested in the last minute different
     *  keys and `number`. If it is not specified, returns only keys that have been requested at >= ~10% of requests in
     *  the last period.
     *
     *  E.g. if `number` is not specified and there are not a single key which has been requested at >= ~10% of requests
     *  in the last period, then the returned vector is empty.
     *
     *  Time complexity: O(1).
     */
    std::vector<Key> GetTopKKeys(int number = 0);

private:
    /**
     *  @brief  Handy way of keeping bucket information (instead of using std::pair/std::tuple)
     */
    struct BucketInfo {
        std::chrono::system_clock::time_point created_at;
        std::map<Key, int64_t, Compare> bucket_data;
        int64_t add_new_key_count;

        explicit BucketInfo(std::chrono::system_clock::time_point created_at);
    };

    const std::chrono::duration<double> control_time_;
    // The oldest bucket always includes statistics for the whole control_time and a bit more,
    // but no more than full_control_time_
    const std::chrono::duration<double> full_control_time_;
    const size_t buckets_count_;
    const size_t bucket_size_;
    const double share_very_frequent_;

    void DeleteOldAddNewBuckets();

    // Three functions from the article "Frequency Estimation" (look README.md)
    inline bool IncrementCounter(std::map<Key, int64_t> &bucket_data, const Key &key);

    inline bool CreateNewCounter(std::map<Key, int64_t> &bucket_data, const Key &key);

    inline void DecreaseAllCounters(std::map<Key, int64_t> &bucket_data, const Key &key);

    void AddKeyToBucket(BucketInfo &bucket_info, const Key &key);

    void AddKeyToBuckets(const Key &key);

    std::vector<std::pair<int64_t, Key>> GetBucketSortedByFrequencyKeys(std::map<Key, int64_t> &bucket_data);

    std::vector<Key>
    WeedOutExtraKeysAndInfo(const std::vector<std::pair<int64_t, Key>> &bucket_vector, int64_t n, int number = 0) const;

    std::list<BucketInfo> buckets_;
};

template<typename Key, typename Compare>
FrequencyEstimationAnalyzer<Key, Compare>::FrequencyEstimationAnalyzer(
        const std::chrono::duration<double> control_time, const double share_very_frequent, const size_t num_buckets,
        const size_t bucket_size)
        : control_time_(control_time),
          full_control_time_(control_time / num_buckets * (num_buckets + 1)),
          buckets_count_(num_buckets + 1), bucket_size_(bucket_size),
          share_very_frequent_(share_very_frequent),
          buckets_() {};

template<typename Key, typename Compare>
void FrequencyEstimationAnalyzer<Key, Compare>::AddKey(const Key &key) {
    DeleteOldAddNewBuckets();
    AddKeyToBuckets(key);
}

template<typename Key, typename Compare>
std::vector<Key>
FrequencyEstimationAnalyzer<Key, Compare>::GetTopKKeys(const int number) {
    DeleteOldAddNewBuckets();
    const std::vector<std::pair<int64_t, Key>> very_frequent_keys =
            GetBucketSortedByFrequencyKeys(buckets_.front().bucket_data);
    return WeedOutExtraKeysAndInfo(very_frequent_keys, buckets_.front().add_new_key_count, number);
}

template<typename Key, typename Compare>
void FrequencyEstimationAnalyzer<Key, Compare>::DeleteOldAddNewBuckets() {
    const std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    while (!buckets_.empty() && now - buckets_.front().created_at > full_control_time_) {
        buckets_.pop_front();
    }

    if (now - buckets_.back().created_at > full_control_time_ / buckets_count_) {
        buckets_.push_back(BucketInfo(now));
    }
}

template<typename Key, typename Compare>
bool FrequencyEstimationAnalyzer<Key, Compare>::IncrementCounter(
        std::map<Key, int64_t> &bucket_data, const Key &key) {
    auto it = bucket_data.find(key);
    if (it != bucket_data.end()) {
        ++it->second;
        return true;
    }
    return false;
}

template<typename Key, typename Compare>
bool FrequencyEstimationAnalyzer<Key, Compare>::CreateNewCounter(
        std::map<Key, int64_t> &bucket_data, const Key &key) {
    if (bucket_data.size() < bucket_size_) {
        bucket_data.emplace(key, 1);
        return true;
    } else {
        for (auto it = bucket_data.begin(); it != bucket_data.end(); ++it) {
            if (it->second == 0) {
                bucket_data.erase(it);
                bucket_data[key] = 1;
                return true;
            }
        }
    }

    return false;
}

template<typename Key, typename Compare>
void FrequencyEstimationAnalyzer<Key, Compare>::DecreaseAllCounters(
        std::map<Key, int64_t> &bucket_data, const Key &key) {
    for (auto it = bucket_data.begin(); it != bucket_data.end(); ++it) {
        if (it->second > 0) {
            it->second--;
        }
    }
}

template<typename Key, typename Compare>
void FrequencyEstimationAnalyzer<Key, Compare>::AddKeyToBucket(
        FrequencyEstimationAnalyzer<Key, Compare>::BucketInfo &bucket_info,
        const Key &key) {
    if (!IncrementCounter(bucket_info.bucket_data, key)) {
        if (!CreateNewCounter(bucket_info.bucket_data, key)) {
            DecreaseAllCounters(bucket_info.bucket_data, key);
        }
    }
}

template<typename Key, typename Compare>
void FrequencyEstimationAnalyzer<Key, Compare>::AddKeyToBuckets(const Key &key) {
    for (auto it = buckets_.begin(); it != buckets_.end(); ++it) {
        it->add_new_key_count++;
        AddKeyToBucket(*it, key);
    }
}

template<typename Key, typename Compare>
std::vector<std::pair<int64_t, Key>>
FrequencyEstimationAnalyzer<Key, Compare>::GetBucketSortedByFrequencyKeys(
        std::map<Key, int64_t> &bucket_data) {
    std::vector<std::pair<int64_t, Key>> bucket_vector;
    for (auto it = bucket_data.begin(); it != bucket_data.end(); ++it) {
        bucket_vector.emplace_back(it->second, it->first);
    }
    std::sort(bucket_vector.begin(), bucket_vector.end(),
              [](std::pair<int64_t, Key> &left, std::pair<int64_t, Key> &right) {
                  return left.first > right.first;
              });
    return bucket_vector;
}

template<typename Key, typename Compare>
std::vector<Key>
FrequencyEstimationAnalyzer<Key, Compare>::WeedOutExtraKeysAndInfo(
        const std::vector<std::pair<int64_t, Key>> &bucket_vector, int64_t n, int number) const {
    std::vector<Key> result;
    const double min_num = floor((double) n * share_very_frequent_) -
                           ceil((double) n * (1 - share_very_frequent_) / (double) bucket_size_) - 2;
    if (number == 0) {
        for (size_t i = 0; i < bucket_vector.size() && bucket_vector[i].first >= min_num; ++i) {
            result.push_back(bucket_vector[i].second);
        }
    } else {
        for (size_t i = 0; i < bucket_vector.size() && i < number; ++i) {
            result.push_back(bucket_vector[i].second);
        }
    }
    return result;
}

template<typename Key, typename Compare>
FrequencyEstimationAnalyzer<Key, Compare>::BucketInfo::BucketInfo(
        std::chrono::system_clock::time_point created_at) : created_at(created_at),
                                                            bucket_data(),
                                                            add_new_key_count(0) {};

#endif //VKTEST_FREQUENCY_ESTIMATION_ANALYZER_H

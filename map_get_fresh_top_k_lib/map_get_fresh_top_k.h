// MapGetFreshTopK implementation -*- C++ -*-

// Made especially for VK Team by Nikita Lisovetin (vk: vk.com/nikitalisovetin, e-mail: nik-lisovetin@ya.ru)

#ifndef VKTEST_MAPGETFRESHTOPK_H
#define VKTEST_MAPGETFRESHTOPK_H

#include <string>
#include <map>
#include <chrono>
#include <list>
#include <vector>
#include <utility>
#include <algorithm>
#include <iostream>
#include <exception>

#include "frequency_estimation_analyzer.h"

/**
 *  @brief A modification of standard STL map with the additional "show keys asked most
 *  frequently for the last period function.
 *
 *  @tparam Key  Type of key objects, defaults to std::string
 *  @tparam Tp  Type of mapped objects, defaults to std::string
 *  @tparam Compare  Comparison function object type, defaults to less<Key>.
 *  @tparam Alloc  Allocator type, defaults to allocator<pair<const Key, Tp>.
 *
 *  By default accordingly to the given task this map is string->string,
 *  additional function get_top_k shows all keys asked most frequently
 *  accordingly to the "frequency estimation algorithm" (check README.md)
 *  for 10% and for the last minute by default. If no argument is provided,
 *  then shows only keys which were in >= ~10% of requests. If the argument
 *  `number` is provided, then tries to show `number` top keys.
 */
template<typename Key = std::string, typename Tp = std::string, typename Compare = std::less<Key>, typename Alloc = std::allocator<std::pair<const Key, Tp>>>
class MapGetFreshTopK {
public:
    /**
     *  @brief Duplicate key request frequency analyzer constructor.
     *
     *  @param control_time  Timespan, defaults to 60 seconds.
     *  @param share_to_be_very_frequent  Share of requests for keys to be
     *  considered as "very frequent", defaults to 0.1 (10%).
     *  @param num_buckets  Amount of buckets, defaults to 12.
     *  @param bucket_size  Size of each bucket, defaults to 54.
     */
    explicit MapGetFreshTopK(std::chrono::duration<double> control_time = std::chrono::seconds(60),
                             double share_to_be_very_frequent = 0.1, size_t num_buckets = 12,
                             size_t bucket_size = 54);

    /**
     *  @brief  Access to %map data.
     *  @param  key  The key for which data should be retrieved.
     *  @return  A reference to the data of the (key, data) %pair.
     *
     *  Returns data associated with the key `key`. If the key
     *  does not exist, a pair with that key is created using
     *  default values, which is then returned.
     *
     *  Time complexity: O(log(n)), where n - size of the map
     */
    Tp &get(const Key &key);

    /**
     *  @brief  Add or change %map data.
     *  @param  key  The key for which data should be added or changed. If data
     *          whose key is equivalent is present, then change it.
     *  @param  value  The value which should be associated with the `key`.
     *
     *  If the key does not exist, a pair with that key is created using `value`.
     *  Else change the value of data associated with the key `key` to the data
     *  `value`.
     *
     *  Time complexity: O(log(n)), where n is the size of the map.
     */
    void set(const Key &key, const Tp &value);

    /**
     *  @brief  Print very frequently asked keys.
     *  @param number  Number of the requested top by frequency of requests in the last period keys.
     *  @return  Vector with frequently asked keys for the last minute.
     *
     *  Returned vector size can be different. If `number` is specified, returns min of requested in the last minute
     *  different keys and `number`, but the top can be wrong, because math theory is designed for keys requested in
     *  >= ~10% of requests. If it is not specified, returns only keys that have been requested at >= ~10% of
     *  requests in the last period accordingly to the "frequency estimation algorithm".
     *
     *  E.g. if `number` is not specified and there are not a single key which has been requested at >= ~10% of requests
     *  in the last period, then the returned vector is empty.
     *
     *  Time complexity: O(1).
     */
    std::vector<Key> get_top_k(const size_t number = 0);

private:

    std::map<Key, Tp, Compare, Alloc> map_;
    FrequencyEstimationAnalyzer<Key, Compare> analyzer_;
};

template<typename Key, typename Tp, typename Compare, typename Alloc>
MapGetFreshTopK<Key, Tp, Compare, Alloc>::MapGetFreshTopK(
        const std::chrono::duration<double> control_time, const double share_to_be_very_frequent,
        const size_t num_buckets,
        const size_t bucket_size): analyzer_(
        FrequencyEstimationAnalyzer<Key, Compare>(control_time, share_to_be_very_frequent, num_buckets, bucket_size)) {
};

template<typename Key, typename Tp, typename Compare, typename Alloc>
Tp &MapGetFreshTopK<Key, Tp, Compare, Alloc>::get(const Key &key) {
    // #sleep well at night
    try {
        analyzer_.AddKey(key);
    } catch (std::exception &e) {
        // don't try again add this key - an exception is a rare situation, this action doesn't affect statistics
    }

    return map_[key];
}

template<typename Key, typename Tp, typename Compare, typename Alloc>
void MapGetFreshTopK<Key, Tp, Compare, Alloc>::set(const Key &key, const Tp &value) {
    map_[key] = value;

    // #sleep well at night
    try {
        analyzer_.AddKey(key);
    } catch (std::exception &e) {
        // don't try again add this key - an exception is a rare situation, this action doesn't affect statistics
    }
}

template<typename Key, typename Tp, typename Compare, typename Alloc>
std::vector<Key>
MapGetFreshTopK<Key, Tp, Compare, Alloc>::get_top_k(size_t number) {
    // #sleep well at night
    try {
        return analyzer_.GetTopKKeys(number);
    } catch (std::exception &e) {
        return std::vector<Key>();
    }
}

#endif //VKTEST_MAPGETFRESHTOPK_H

// Utility functions for testing of MapWithGetVeryFrequnt implementation -*- C++ -*-

// Made especially for VK Team by Nikita Lisovetin (vk: vk.com/nikitalisovetin, e-mail: nik-lisovetin@ya.ru)

#ifndef VKTEST_UTILITY_FUNCTIONS_H
#define VKTEST_UTILITY_FUNCTIONS_H

#include "gtest/gtest.h"
#include "map_get_fresh_top_k.h"

#include <math.h>

#include <set>
#include <ctime>
#include <chrono>
#include <cstdlib>
#include <random>
#include <string>
#include <algorithm>
// uncomment to disable assert()
// #define NDEBUG
#include <cassert>
#include <thread>

// Use (void) to silent unused warnings.
#define assertm(exp, msg) assert(((void)msg, exp))

// For google test custom messages
#define GTEST_COUT std::cerr << "[          ] [ INFO ]"

#include "accurate_frequency_analyzer.h"

/**
 *  @brief std::cout vector (expected and actual).
 */
std::string StringifyVector(std::vector<std::string> &vec) {
    std::string result = "";
    for (size_t i = 0; i < vec.size(); ++i) {
        result += vec[i] + ' ';
    }
    return result;
}

/**
 *  @brief Generates random keys and values.
 *
 *  @param is_human_chars  Determines if chars used by human or any from [0,256) ASCII, defaults to true.
 */
std::string GenerateRandomString(size_t length, const bool is_human_chars = true) {
    std::string charmap;

    if (is_human_chars) {
        charmap = "0123456789!@#$%^&*ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    } else {
        for (int i = 0; i < 256; ++i) {
            charmap += char(i);
        }
    }

    const size_t charmapLength = charmap.length();
    auto generator = [&]() { return charmap[rand() % charmapLength]; };
    std::string result;
    result.reserve(length);
    generate_n(back_inserter(result), length, generator);
    return result;
}

/**
 *  @brief To check if the expected vector of frequently asked keys inside of the actual (our) vector.
 */
template<typename T>
bool IsOneVectorInAnother(std::vector<T> small, std::vector<T> big) {
    std::set<T> set_big(big.begin(), big.end());

    for (const auto &element : small) {
        if (set_big.find(element) == set_big.end()) {
            return false;
        }
    }

    return true;
}

/**
 *  @brief Support class for holding hot_keys with random probabilities in a vector.
 *
 *  Probabilities are values between 0 and 1. When they are in the vector, the probability of the last key is 1.
 *  Each key[i] is associated with a segment (key[i - 1].probability || 0 if i==1; key[i].probability]. Some keys may
 *  not be hot, but num_hot_keys is typically <=10 and general hot_rate ~0.5 (50%).
 */
struct KeyAndProbability {
public:
    const std::string key;
    const double probability;

    KeyAndProbability(const std::string key, const double probability) : key(key), probability(probability) {};

    KeyAndProbability(const double probability) : probability(probability) {};

    bool operator<(const KeyAndProbability &other) {
        return probability < other.probability;
    }
};

/**
 *  @brief Base 'simulate requests' function with different behavior on specific time period.
 *
 *  @param map  Our map.
 *  @param analyzer  Our analyzer used for checking our map.
 *  @param global_get_num  Global number of get requests used for checking rate of made mistakes by our map.
 *  @param mistakes  Global number of mistakes made by our map (1 mistake = 1 incorrect get result).
 *  @param mistakes_rate_sum  Sum of (mistakes_rate at the moments of mistakes).
 *  @param hotkeys  Vector of hotkeys with probabilities. Check @a KeyAndProbability class for details.
 *  @param keys  Vector of keys. If empty() == true, then we generate random keys.
 *  @param hot_rate  Total frequency rate of @a hotkeys.
 *  @param micro_get_period  Time interval between get requests, defaults to 1000 microseconds.
 *  @param duration  Duration of this operation, defaults to 5 sec.
 *  @param is_hotkey  Set hotkeys or do not, defaults to false.
 *  @param is_only_get  Only get requests or also set some keys, defaults to false.
 */
void
SetHotkeysOrNotOrOnlyGet(MapGetFreshTopK<> &map,
                         AccurateFrequencyAnalyzer<> &analyzer,
                         int64_t &global_get_num,
                         int64_t &mistakes,
                         std::vector<KeyAndProbability> &hotkeys,
                         std::vector<std::string> &keys,
                         const double hot_rate = 0.01,
                         std::chrono::duration<double> micro_get_period = std::chrono::microseconds(1000),
                         const std::chrono::duration<double> duration = std::chrono::milliseconds(5000),
                         const bool is_hotkey = false,
                         const bool is_only_get = false
) {
    std::random_device r;
    std::default_random_engine random_engine(r());
    std::uniform_real_distribution<double> uniform_dist(double(0), double(1));
    std::uniform_int_distribution<int> uniform_dist_keys(0, static_cast<int>(keys.size()) - 1);
    std::string key, val;

    std::vector<std::string> result, expected;

    bool is_mistake;
    auto start_time = std::chrono::system_clock::now();
    auto last_get_time = std::chrono::system_clock::now();
    double generated_rate_0_1;

    const bool is_keys_provided = !keys.empty();
    while (std::chrono::system_clock::now() - start_time < duration) {
        if (!is_only_get) {
            if (!is_hotkey) {
                generated_rate_0_1 = uniform_dist(random_engine);
                if (generated_rate_0_1 < hot_rate) {
                    key = std::lower_bound(hotkeys.begin(), hotkeys.end(), KeyAndProbability(generated_rate_0_1))->key;
                } else {
                    if (!is_keys_provided) {
                        key = GenerateRandomString(10);
                    } else {
                        key = keys[uniform_dist_keys(random_engine)];
                    }
                }
            } else {
                if (!is_keys_provided) {
                    key = GenerateRandomString(10);
                } else {
                    key = keys[uniform_dist_keys(random_engine)];
                }
            }
            val = GenerateRandomString(10);
            map.set(key, val);
            analyzer.add(key);
        }

        if (std::chrono::system_clock::now() - last_get_time > micro_get_period) {
            last_get_time = std::chrono::system_clock::now();
            global_get_num++;
            result = map.get_top_k();
            expected = analyzer.GetActualTop();

            is_mistake = !IsOneVectorInAnother(expected, result);
            if (is_mistake) {
                mistakes++;
            }
        }
    }
}

void GenerateHotKeys(std::vector<KeyAndProbability> &hotkeys, const size_t num_hotkeys = 1) {
    std::random_device r;
    std::default_random_engine random_engine_0_1(r());
    std::uniform_real_distribution<double> uniform_dist(double(0), double(1));
    double remaining_hot_rate = 1;
    double key_rate;
    for (size_t i = 0; i < num_hotkeys - 1; ++i) {
        key_rate = uniform_dist(random_engine_0_1);
        remaining_hot_rate -= key_rate;
        hotkeys.emplace_back(GenerateRandomString(10), 1 - remaining_hot_rate);
    }
    hotkeys.emplace_back(GenerateRandomString(10), 1);
}

bool HotkeysAtTheBeginningOrEndOnlyOneGet(const double hot_rate = 0.11,
                                          const int64_t microseconds_get_period = 1000,
                                          const int64_t milliseconds_set_hotkey = 500,
                                          const int64_t milliseconds_set_not_hotkey = 500,
                                          const bool is_beginning = true,
                                          const size_t num_hotkeys = 1,
                                          const size_t num_keys = 0) {

    assertm(milliseconds_set_hotkey + milliseconds_set_not_hotkey < 1000,
            "milliseconds_set_hotkey + milliseconds_set_not_hotkey should be < 1000, because the purpose of this test is to make ONE GET and make sure the EXPECTED is the ACTUAL (actual works always with a margin and this margin can be a reason for unequality with expected, this test does not test this margin)");

    const auto get_period = std::chrono::microseconds(microseconds_get_period);
    const auto set_hotkey_duration = std::chrono::milliseconds(milliseconds_set_hotkey);
    const auto set_not_hotkey_duration = std::chrono::milliseconds(milliseconds_set_not_hotkey);

    std::vector<KeyAndProbability> hotkeys;

    GenerateHotKeys(hotkeys, num_hotkeys);

    std::vector<std::string> keys;
    keys.resize(num_keys, GenerateRandomString(10));

    MapGetFreshTopK<> map(std::chrono::seconds(1), 0.1, 12, 54);
    AccurateFrequencyAnalyzer<> analyzer;

    int64_t global_get_num = 0;
    int64_t mistakes = 0;
    if (is_beginning) {
        SetHotkeysOrNotOrOnlyGet(map, analyzer, global_get_num, mistakes, hotkeys, keys,
                                 hot_rate, get_period, set_hotkey_duration, true, false);
        SetHotkeysOrNotOrOnlyGet(map, analyzer, global_get_num, mistakes, hotkeys, keys,
                                 hot_rate, get_period, set_not_hotkey_duration, false, false);
    } else {
        SetHotkeysOrNotOrOnlyGet(map, analyzer, global_get_num, mistakes, hotkeys, keys,
                                 hot_rate, get_period, set_not_hotkey_duration, false, false);
        SetHotkeysOrNotOrOnlyGet(map, analyzer, global_get_num, mistakes, hotkeys, keys,
                                 hot_rate, get_period, set_hotkey_duration, true, false);
    }

    auto result = map.get_top_k();
    auto expected = analyzer.GetActualTop();

    bool is_mistake = !IsOneVectorInAnother(expected, result);

    if (is_mistake) {
        GTEST_COUT << std::endl << "Actual: " << StringifyVector(result) << std::endl
                   << "Expected: <=" << StringifyVector(expected);
        return false;
    } else {
        return true;
    }
}

bool HotkeysAtTheBeginningOrEnd(const double hot_rate = 0.11,
                                const int64_t microseconds_get_period = 1000,
                                const int64_t milliseconds_set_hotkey = 5000,
                                const int64_t milliseconds_set_not_hotkey = 5000,
                                const int64_t milliseconds_get_in_end = 1000,
                                const bool is_beginning = true,
                                const bool increased_accuracy = false,
                                const size_t num_hotkeys = 1,
                                const size_t num_keys = 0) {

    const auto get_period = std::chrono::microseconds(microseconds_get_period);
    const auto set_hotkey_duration = std::chrono::milliseconds(milliseconds_set_hotkey);
    const auto set_not_hotkey_duration = std::chrono::milliseconds(milliseconds_set_not_hotkey);
    const auto get_in_end_duration = std::chrono::milliseconds(milliseconds_get_in_end);

    std::string hotkey = GenerateRandomString(10);

    std::vector<KeyAndProbability> hotkeys;

    GenerateHotKeys(hotkeys, num_hotkeys);

    std::vector<std::string> keys;
    keys.resize(num_keys, GenerateRandomString(10));

    MapGetFreshTopK<> map(std::chrono::seconds(1), 0.1, increased_accuracy ? 20 : 12,
                                 increased_accuracy ? 100 : 54);
    AccurateFrequencyAnalyzer<> analyzer;

    int64_t global_get_num = 0;
    int64_t mistakes = 0;
    if (is_beginning) {
        SetHotkeysOrNotOrOnlyGet(map, analyzer, global_get_num, mistakes, hotkeys, keys,
                                 hot_rate, get_period, set_hotkey_duration, true, false);
        SetHotkeysOrNotOrOnlyGet(map, analyzer, global_get_num, mistakes, hotkeys, keys,
                                 hot_rate, get_period, set_not_hotkey_duration, false, false);
    } else {
        SetHotkeysOrNotOrOnlyGet(map, analyzer, global_get_num, mistakes, hotkeys, keys,
                                 hot_rate, get_period, set_not_hotkey_duration, false, false);
        SetHotkeysOrNotOrOnlyGet(map, analyzer, global_get_num, mistakes, hotkeys, keys,
                                 hot_rate, get_period, set_hotkey_duration, true, false);
    }

    SetHotkeysOrNotOrOnlyGet(map, analyzer, global_get_num, mistakes, hotkeys, keys, hot_rate,
                             get_period, get_in_end_duration, false, true);

    double average_mistakes_rate = (double) mistakes / (double) global_get_num;

    if (average_mistakes_rate > 0.01) {
        GTEST_COUT << std::endl << "Actual average rate of mistakes: " << average_mistakes_rate << std::endl
                   << "Expected average rate of mistakes: <= " << 0.01;
        return false;
    } else {
        return true;
    }
}

struct HotkeyBegOrEndParameters {
public:
    double hot_rate = 0.11;
    int64_t microseconds_get_period = 1000;
    int64_t milliseconds_set_hotkey = 5000;
    int64_t milliseconds_set_not_hotkey = 5000;
    int64_t milliseconds_get_in_end = 1000;
    bool is_beginning = true;
    bool increased_accuracy = false;
    size_t num_hotkeys = 1;
    size_t num_keys = 0;

    HotkeyBegOrEndParameters(double hot_rate, int64_t microseconds_get_period, int64_t milliseconds_set_hotkey,
                             int64_t milliseconds_set_not_hotkey, int64_t milliseconds_get_in_end, bool is_beginning,
                             bool increased_accuracy = false)
            : hot_rate(hot_rate), microseconds_get_period(microseconds_get_period),
              milliseconds_set_hotkey(milliseconds_set_hotkey),
              milliseconds_set_not_hotkey(milliseconds_set_not_hotkey),
              milliseconds_get_in_end(milliseconds_get_in_end),
              is_beginning(is_beginning), increased_accuracy(increased_accuracy) {}

    HotkeyBegOrEndParameters(const bool is_random, const bool is_many_hotkeys = false) {
        if (is_random) {
            std::random_device r;
            std::default_random_engine random_engine(r());
            std::uniform_real_distribution<double> uniform_dist(double(0), double(1));
            std::uniform_int_distribution<size_t> uniform_dist_get_period(500, 20000);
            std::uniform_int_distribution<size_t> uniform_dist_num_hotkeys(1, 10);
            std::uniform_int_distribution<size_t> uniform_dist_num_keys(0, 1000);

            hot_rate = is_many_hotkeys ? 0.5 : uniform_dist(random_engine);
            microseconds_get_period = uniform_dist_get_period(random_engine);
            milliseconds_set_hotkey = static_cast<int64_t>(round(uniform_dist(random_engine) * 5000 + 50));
            milliseconds_set_not_hotkey = static_cast<int64_t>(round(uniform_dist(random_engine) * 5000 + 50));
            milliseconds_get_in_end = static_cast<int64_t>(round(uniform_dist(random_engine) * 5000));
            is_beginning = uniform_dist(random_engine) < 0.5;
            increased_accuracy = true;
            num_hotkeys = uniform_dist_num_hotkeys(random_engine);
            num_keys = uniform_dist_num_keys(random_engine);
        }
    }
};

bool TestSpecificParametersBeginning() {

    const std::vector<HotkeyBegOrEndParameters> vector_ask{
            HotkeyBegOrEndParameters(0.05, 1000, 500, 0, 0, true, true),
            HotkeyBegOrEndParameters(0.05, 1000, 500, 500, 0, true, true),
            HotkeyBegOrEndParameters(0.05, 1000, 500, 500, 500, true, true),
            HotkeyBegOrEndParameters(0.05, 1000, 0, 500, 500, true, true),
            HotkeyBegOrEndParameters(0.05, 1000, 0, 0, 500, true, true),
            HotkeyBegOrEndParameters(0.05, 1000, 500, 500, 500, true, true),

            HotkeyBegOrEndParameters(0.05, 1000, 1200, 0, 0, true, true),
            HotkeyBegOrEndParameters(0.05, 1000, 1200, 1200, 0, true, true),
            HotkeyBegOrEndParameters(0.05, 1000, 1200, 1200, 1200, true, true),
            HotkeyBegOrEndParameters(0.05, 1000, 0, 1200, 1200, true, true),
            HotkeyBegOrEndParameters(0.05, 1000, 0, 0, 1200, true, true),
            HotkeyBegOrEndParameters(0.05, 1000, 1200, 1200, 1200, true, true),

            HotkeyBegOrEndParameters(0.05, 1000, 3000, 0, 0, true, true),
            HotkeyBegOrEndParameters(0.05, 1000, 3000, 3000, 0, true, true),
            HotkeyBegOrEndParameters(0.05, 1000, 3000, 3000, 3000, true, true),
            HotkeyBegOrEndParameters(0.05, 1000, 0, 3000, 3000, true, true),
            HotkeyBegOrEndParameters(0.05, 1000, 0, 0, 3000, true, true),
            HotkeyBegOrEndParameters(0.05, 1000, 3000, 3000, 3000, true, true),


            HotkeyBegOrEndParameters(0.105, 1000, 500, 0, 0, true, true),
            HotkeyBegOrEndParameters(0.105, 1000, 500, 500, 0, true, true),
            HotkeyBegOrEndParameters(0.105, 1000, 500, 500, 500, true, true),
            HotkeyBegOrEndParameters(0.105, 1000, 0, 500, 500, true, true),
            HotkeyBegOrEndParameters(0.105, 1000, 0, 0, 500, true, true),
            HotkeyBegOrEndParameters(0.105, 1000, 500, 500, 500, true, true),

            HotkeyBegOrEndParameters(0.105, 1000, 1200, 0, 0, true, true),
            HotkeyBegOrEndParameters(0.105, 1000, 1200, 1200, 0, true, true),
            HotkeyBegOrEndParameters(0.105, 1000, 1200, 1200, 1200, true, true),
            HotkeyBegOrEndParameters(0.105, 1000, 0, 1200, 1200, true, true),
            HotkeyBegOrEndParameters(0.105, 1000, 0, 0, 1200, true, true),
            HotkeyBegOrEndParameters(0.105, 1000, 1200, 1200, 1200, true, true),

            HotkeyBegOrEndParameters(0.105, 1000, 3000, 0, 0, true, true),
            HotkeyBegOrEndParameters(0.105, 1000, 3000, 3000, 0, true, true),
            HotkeyBegOrEndParameters(0.105, 1000, 3000, 3000, 3000, true, true),
            HotkeyBegOrEndParameters(0.105, 1000, 0, 3000, 3000, true, true),
            HotkeyBegOrEndParameters(0.105, 1000, 0, 0, 3000, true, true),
            HotkeyBegOrEndParameters(0.105, 1000, 3000, 3000, 3000, true, true),


            HotkeyBegOrEndParameters(0.15, 1000, 500, 0, 0, true, true),
            HotkeyBegOrEndParameters(0.15, 1000, 500, 500, 0, true, true),
            HotkeyBegOrEndParameters(0.15, 1000, 500, 500, 500, true, true),
            HotkeyBegOrEndParameters(0.15, 1000, 0, 500, 500, true, true),
            HotkeyBegOrEndParameters(0.15, 1000, 0, 0, 500, true, true),
            HotkeyBegOrEndParameters(0.15, 1000, 500, 500, 500, true, true),

            HotkeyBegOrEndParameters(0.15, 1000, 1200, 0, 0, true, true),
            HotkeyBegOrEndParameters(0.15, 1000, 1200, 1200, 0, true, true),
            HotkeyBegOrEndParameters(0.15, 1000, 1200, 1200, 1200, true, true),
            HotkeyBegOrEndParameters(0.15, 1000, 0, 1200, 1200, true, true),
            HotkeyBegOrEndParameters(0.15, 1000, 0, 0, 1200, true, true),
            HotkeyBegOrEndParameters(0.15, 1000, 1200, 1200, 1200, true, true),

            HotkeyBegOrEndParameters(0.15, 1000, 3000, 0, 0, true, true),
            HotkeyBegOrEndParameters(0.15, 1000, 3000, 3000, 0, true, true),
            HotkeyBegOrEndParameters(0.15, 1000, 3000, 3000, 3000, true, true),
            HotkeyBegOrEndParameters(0.15, 1000, 0, 3000, 3000, true, true),
            HotkeyBegOrEndParameters(0.15, 1000, 0, 0, 3000, true, true),
            HotkeyBegOrEndParameters(0.15, 1000, 3000, 3000, 3000, true, true),
    };

    for (const auto &i : vector_ask) {
        if (!HotkeysAtTheBeginningOrEnd(i.hot_rate,
                                        i.microseconds_get_period,
                                        i.milliseconds_set_hotkey,
                                        i.milliseconds_set_not_hotkey,
                                        i.milliseconds_get_in_end,
                                        i.is_beginning,
                                        i.increased_accuracy)) {

            GTEST_COUT << std::endl << "Failed test with following parameters:" << std::endl
                       << "hot_rate " << i.hot_rate << std::endl
                       << "microseconds_get_period " << i.microseconds_get_period << std::endl
                       << "milliseconds_set_hotkey " << i.milliseconds_set_hotkey << std::endl
                       << "milliseconds_set_not_hotkey " << i.milliseconds_set_not_hotkey << std::endl
                       << "milliseconds_get_in_end " << i.milliseconds_get_in_end << std::endl
                       << "is_beginning " << i.is_beginning << std::endl
                       << "increased_accuracy " << i.increased_accuracy << std::endl;

            return false;
        }
    }
    return true;
}


bool TestSpecificParametersEnd() {

    const std::vector<HotkeyBegOrEndParameters> vector_ask{
            HotkeyBegOrEndParameters(0.05, 1000, 500, 0, 0, false, true),
            HotkeyBegOrEndParameters(0.05, 1000, 500, 500, 0, false, true),
            HotkeyBegOrEndParameters(0.05, 1000, 500, 500, 500, false, true),
            HotkeyBegOrEndParameters(0.05, 1000, 0, 500, 500, false, true),
            HotkeyBegOrEndParameters(0.05, 1000, 0, 0, 500, false, true),
            HotkeyBegOrEndParameters(0.05, 1000, 500, 500, 500, false, true),

            HotkeyBegOrEndParameters(0.05, 1000, 1200, 0, 0, false, true),
            HotkeyBegOrEndParameters(0.05, 1000, 1200, 1200, 0, false, true),
            HotkeyBegOrEndParameters(0.05, 1000, 1200, 1200, 1200, false, true),
            HotkeyBegOrEndParameters(0.05, 1000, 0, 1200, 1200, false, true),
            HotkeyBegOrEndParameters(0.05, 1000, 0, 0, 1200, false, true),
            HotkeyBegOrEndParameters(0.05, 1000, 1200, 1200, 1200, false, true),

            HotkeyBegOrEndParameters(0.05, 1000, 3000, 0, 0, false, true),
            HotkeyBegOrEndParameters(0.05, 1000, 3000, 3000, 0, false, true),
            HotkeyBegOrEndParameters(0.05, 1000, 3000, 3000, 3000, false, true),
            HotkeyBegOrEndParameters(0.05, 1000, 0, 3000, 3000, false, true),
            HotkeyBegOrEndParameters(0.05, 1000, 0, 0, 3000, false, true),
            HotkeyBegOrEndParameters(0.05, 1000, 3000, 3000, 3000, false, true),


            HotkeyBegOrEndParameters(0.105, 1000, 500, 0, 0, false, true),
            HotkeyBegOrEndParameters(0.105, 1000, 500, 500, 0, false, true),
            HotkeyBegOrEndParameters(0.105, 1000, 500, 500, 500, false, true),
            HotkeyBegOrEndParameters(0.105, 1000, 0, 500, 500, false, true),
            HotkeyBegOrEndParameters(0.105, 1000, 0, 0, 500, false, true),
            HotkeyBegOrEndParameters(0.105, 1000, 500, 500, 500, false, true),

            HotkeyBegOrEndParameters(0.105, 1000, 1200, 0, 0, false, true),
            HotkeyBegOrEndParameters(0.105, 1000, 1200, 1200, 0, false, true),
            HotkeyBegOrEndParameters(0.105, 1000, 1200, 1200, 1200, false, true),
            HotkeyBegOrEndParameters(0.105, 1000, 0, 1200, 1200, false, true),
            HotkeyBegOrEndParameters(0.105, 1000, 0, 0, 1200, false, true),
            HotkeyBegOrEndParameters(0.105, 1000, 1200, 1200, 1200, false, true),

            HotkeyBegOrEndParameters(0.105, 1000, 3000, 0, 0, false, true),
            HotkeyBegOrEndParameters(0.105, 1000, 3000, 3000, 0, false, true),
            HotkeyBegOrEndParameters(0.105, 1000, 3000, 3000, 3000, false, true),
            HotkeyBegOrEndParameters(0.105, 1000, 0, 3000, 3000, false, true),
            HotkeyBegOrEndParameters(0.105, 1000, 0, 0, 3000, false, true),
            HotkeyBegOrEndParameters(0.105, 1000, 3000, 3000, 3000, false, true),


            HotkeyBegOrEndParameters(0.15, 1000, 500, 0, 0, false, true),
            HotkeyBegOrEndParameters(0.15, 1000, 500, 500, 0, false, true),
            HotkeyBegOrEndParameters(0.15, 1000, 500, 500, 500, false, true),
            HotkeyBegOrEndParameters(0.15, 1000, 0, 500, 500, false, true),
            HotkeyBegOrEndParameters(0.15, 1000, 0, 0, 500, false, true),
            HotkeyBegOrEndParameters(0.15, 1000, 500, 500, 500, false, true),

            HotkeyBegOrEndParameters(0.15, 1000, 1200, 0, 0, false, true),
            HotkeyBegOrEndParameters(0.15, 1000, 1200, 1200, 0, false, true),
            HotkeyBegOrEndParameters(0.15, 1000, 1200, 1200, 1200, false, true),
            HotkeyBegOrEndParameters(0.15, 1000, 0, 1200, 1200, false, true),
            HotkeyBegOrEndParameters(0.15, 1000, 0, 0, 1200, false, true),
            HotkeyBegOrEndParameters(0.15, 1000, 1200, 1200, 1200, false, true),

            HotkeyBegOrEndParameters(0.15, 1000, 3000, 0, 0, false, true),
            HotkeyBegOrEndParameters(0.15, 1000, 3000, 3000, 0, false, true),
            HotkeyBegOrEndParameters(0.15, 1000, 3000, 3000, 3000, false, true),
            HotkeyBegOrEndParameters(0.15, 1000, 0, 3000, 3000, false, true),
            HotkeyBegOrEndParameters(0.15, 1000, 0, 0, 3000, false, true),
            HotkeyBegOrEndParameters(0.15, 1000, 3000, 3000, 3000, false, true),
    };

    for (const auto &i : vector_ask) {
        if (!HotkeysAtTheBeginningOrEnd(i.hot_rate,
                                        i.microseconds_get_period,
                                        i.milliseconds_set_hotkey,
                                        i.milliseconds_set_not_hotkey,
                                        i.milliseconds_get_in_end,
                                        i.is_beginning,
                                        i.increased_accuracy)) {

            GTEST_COUT << std::endl << "Failed test with following parameters:" << std::endl
                       << "microseconds_get_period " << i.microseconds_get_period << std::endl
                       << "hot_rate " << i.hot_rate << std::endl
                       << "milliseconds_set_hotkey " << i.milliseconds_set_hotkey << std::endl
                       << "milliseconds_set_not_hotkey " << i.milliseconds_set_not_hotkey << std::endl
                       << "milliseconds_get_in_end " << i.milliseconds_get_in_end << std::endl
                       << "is_beginning " << i.is_beginning << std::endl
                       << "increased_accuracy " << i.increased_accuracy << std::endl;

            return false;
        }
    }
    return true;
}

bool BadTests() {

    const std::vector<HotkeyBegOrEndParameters> vector_ask{
            HotkeyBegOrEndParameters(0.105, 1000, 1000, 1000, 1000, true, true),
            HotkeyBegOrEndParameters(0.105, 1000, 3000, 3000, 3000, true, true),
            HotkeyBegOrEndParameters(0.105, 1000, 1000, 1000, 1000, false, true),
            HotkeyBegOrEndParameters(0.105, 1000, 3000, 3000, 3000, false, true),
    };

    for (const auto &i : vector_ask) {
        if (!HotkeysAtTheBeginningOrEnd(i.hot_rate,
                                        i.microseconds_get_period,
                                        i.milliseconds_set_hotkey,
                                        i.milliseconds_set_not_hotkey,
                                        i.milliseconds_get_in_end,
                                        i.is_beginning,
                                        i.increased_accuracy)) {

            GTEST_COUT << std::endl << "Failed test with following parameters:" << std::endl
                       << "microseconds_get_period " << i.microseconds_get_period << std::endl
                       << "hot_rate " << i.hot_rate << std::endl
                       << "milliseconds_set_hotkey " << i.milliseconds_set_hotkey << std::endl
                       << "milliseconds_set_not_hotkey " << i.milliseconds_set_not_hotkey << std::endl
                       << "milliseconds_get_in_end " << i.milliseconds_get_in_end << std::endl
                       << "is_beginning " << i.is_beginning << std::endl
                       << "increased_accuracy " << i.increased_accuracy << std::endl;

            return false;
        }
    }
    return true;
}

bool TestRandomParametersBeginningOrEnd() {

    const std::vector<HotkeyBegOrEndParameters> vector_ask(50, HotkeyBegOrEndParameters(true));

    for (const auto &i : vector_ask) {
        if (!HotkeysAtTheBeginningOrEnd(i.hot_rate,
                                        i.microseconds_get_period,
                                        i.milliseconds_set_hotkey,
                                        i.milliseconds_set_not_hotkey,
                                        i.milliseconds_get_in_end,
                                        i.is_beginning,
                                        i.increased_accuracy)) {

            GTEST_COUT << std::endl << "Failed test with following parameters:" << std::endl
                       << "microseconds_get_period " << i.microseconds_get_period << std::endl
                       << "hot_rate " << i.hot_rate << std::endl
                       << "milliseconds_set_hotkey " << i.milliseconds_set_hotkey << std::endl
                       << "milliseconds_set_not_hotkey " << i.milliseconds_set_not_hotkey << std::endl
                       << "milliseconds_get_in_end " << i.milliseconds_get_in_end << std::endl
                       << "is_beginning " << i.is_beginning << std::endl
                       << "increased_accuracy " << i.increased_accuracy << std::endl;

            return false;
        }
    }
    return true;
}

bool TestRandomParametersBeginningOrEndManyKeys() {

    const std::vector<HotkeyBegOrEndParameters> vector_ask(50, HotkeyBegOrEndParameters(true, true));

    for (const auto &i : vector_ask) {
        if (!HotkeysAtTheBeginningOrEnd(i.hot_rate,
                                        i.microseconds_get_period,
                                        i.milliseconds_set_hotkey,
                                        i.milliseconds_set_not_hotkey,
                                        i.milliseconds_get_in_end,
                                        i.is_beginning,
                                        i.increased_accuracy,
                                        i.num_hotkeys,
                                        i.num_keys)) {

            GTEST_COUT << std::endl << "Failed test with following parameters:" << std::endl
                       << "microseconds_get_period " << i.microseconds_get_period << std::endl
                       << "hot_rate " << 0.5 << std::endl
                       << "milliseconds_set_hotkey " << i.milliseconds_set_hotkey << std::endl
                       << "milliseconds_set_not_hotkey " << i.milliseconds_set_not_hotkey << std::endl
                       << "milliseconds_get_in_end " << i.milliseconds_get_in_end << std::endl
                       << "is_beginning " << i.is_beginning << std::endl
                       << "increased_accuracy " << i.increased_accuracy << std::endl
                       << "num_hotkeys " << i.num_hotkeys << std::endl
                       << "num_keys " << i.num_keys << std::endl;

            return false;
        }
    }
    return true;
}

#endif //VKTEST_UTILITY_FUNCTIONS_H

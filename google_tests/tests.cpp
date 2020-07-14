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

#include "accurate_frequency_analyzer.h"
#include "utility_functions.h"

// Use (void) to silent unused warnings.
#define assertm(exp, msg) assert(((void)msg, exp))

// For google test custom messages
#define GTEST_COUT std::cerr << "[          ] [ INFO ]"

TEST(small_tests_suite, one_set_one_get) {
    std::string key1 = "key_1";
    std::string val1 = "val_1";

    MapGetFreshTopK<> map(std::chrono::seconds(1), 0.1, 12, 54);;

    map.set(key1, val1);
    auto result = map.get_top_k();

    std::vector<std::string> expected{key1};

    ASSERT_TRUE(IsOneVectorInAnother(expected, result));
}

TEST(small_tests_suite, dozen_set_one_get) {
    const size_t num = 10;

    std::vector<std::string> keys, vals;
    for (size_t i = 0; i < num; ++i) {
        keys.push_back("key_" + std::to_string(i));
        vals.push_back("val_" + std::to_string(i));
    }

    MapGetFreshTopK<> map(std::chrono::seconds(1), 0.1, 12, 54);

    for (size_t i = 0; i < num; ++i) {
        map.set(keys[i], vals[i]);
    }

    auto result = map.get_top_k();

    std::vector<std::string> expected{keys};

    ASSERT_TRUE(IsOneVectorInAnother(expected, result));
}

TEST(small_tests_suite, thousand_set_one_get) {
    const size_t num = 1000;

    std::vector<std::string> keys, vals;
    for (size_t i = 0; i < num; ++i) {
        keys.push_back("key_" + std::to_string(i));
        vals.push_back("val_" + std::to_string(i));
    }

    MapGetFreshTopK<> map(std::chrono::seconds(1), 0.1, 12, 54);

    for (size_t i = 0; i < num; ++i) {
        map.set(keys[i], vals[i]);
    }

    auto result = map.get_top_k();

    std::vector<std::string> expected{};

    ASSERT_TRUE(IsOneVectorInAnother(expected, result));
}

TEST(small_tests_suite, million_set_one_get) {
    const size_t num = 1000000;

    std::vector<std::string> keys, vals;
    for (size_t i = 0; i < num; ++i) {
        keys.push_back("key_" + std::to_string(i));
        vals.push_back("val_" + std::to_string(i));
    }

    MapGetFreshTopK<> map(std::chrono::seconds(1), 0.1, 12, 54);

    for (size_t i = 0; i < num; ++i) {
        map.set(keys[i], vals[i]);
    }

    auto result = map.get_top_k();

    std::vector<std::string> expected{};

    ASSERT_TRUE(IsOneVectorInAnother(expected, result));
}

TEST(small_tests_suite, million_same_set_one_get) {
    const size_t num = 1000000;

    std::vector<std::string> keys, vals;
    for (size_t i = 0; i < num; ++i) {
        keys.emplace_back("key_1");
        vals.push_back("val_" + std::to_string(i));
    }

    MapGetFreshTopK<> map(std::chrono::seconds(1), 0.1, 12, 54);

    for (size_t i = 0; i < num; ++i) {
        map.set(keys[i], vals[i]);
    }

    auto result = map.get_top_k();

    std::vector<std::string> expected{"key_1"};

    ASSERT_TRUE(IsOneVectorInAnother(expected, result));
}

// ONE HOTKEY
// beginning
TEST(one_hotkey_at_the_beginning_one_get_suite, _005hotrate_05shot_0snothot_then_one_get) {
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.05, 1000, 500, 0, true));
}

TEST(one_hotkey_at_the_beginning_one_get_suite, _005hotrate_0shot_05snothot_then_one_get) {
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.05, 1000, 0, 499, true));
}

TEST(one_hotkey_at_the_beginning_one_get_suite, _036hotrate_03shot_07snothot_then_one_get) {
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.36, 1000, 300, 699, true));
}

TEST(one_hotkey_at_the_beginning_one_get_suite, _020hotrate_05shot_05snothot_then_one_get) {
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.20, 1000, 500, 499, true));
}

TEST(one_hotkey_at_the_beginning_one_get_suite, _023hotrate_05shot_05snothot_then_one_get) {
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.23, 1000, 500, 499, true));
}

TEST(one_hotkey_at_the_beginning_one_get_suite, _100hotrate_012shot_088snothot_then_one_get) {
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(1, 1000, 120, 879, true));
}

// end
TEST(one_hotkey_at_the_end_one_get_suite, _005hotrate_05shot_0snothot_then_one_get) {
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.05, 1000, 500, 0, false));
}

TEST(one_hotkey_at_the_end_one_get_suite, _005hotrate_0shot_05snothot_then_one_get) {
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.05, 1000, 0, 499, false));
}

TEST(one_hotkey_at_the_end_one_get_suite, _036hotrate_03shot_07snothot_then_one_get) {
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.36, 1000, 300, 699, false));
}

TEST(one_hotkey_at_the_end_one_get_suite, _020hotrate_05shot_05snothot_then_one_get) {
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.20, 1000, 500, 499, false));
}

TEST(one_hotkey_at_the_end_one_get_suite, _023hotrate_05shot_05snothot_then_one_get) {
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.23, 1000, 500, 499, false));
}

TEST(one_hotkey_at_the_end_one_get_suite, _100hotrate_012shot_088snothot_then_one_get) {
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(1, 1000, 120, 879, false));
}


// MANY HOTKEYS
// beginning
TEST(many_hotkeys_at_the_beginning_one_get_suite, _050hotrate_05shot_0snothot_then_one_get) {
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 500, 0, true, 3, 10));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 500, 0, true, 3, 100));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 500, 0, true, 3, 1000));

    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 500, 0, true, 7, 10));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 500, 0, true, 7, 100));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 500, 0, true, 7, 1000));

    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 500, 0, true, 10, 10));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 500, 0, true, 10, 100));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 500, 0, true, 10, 1000));
}

TEST(many_hotkeys_at_the_beginning_one_get_suite, _050hotrate_05shot_05snothot_then_one_get) {
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 500, 499, true, 3, 10));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 500, 499, true, 3, 100));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 500, 499, true, 3, 1000));

    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 500, 499, true, 7, 10));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 500, 499, true, 7, 100));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 500, 499, true, 7, 1000));

    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 500, 499, true, 10, 10));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 500, 499, true, 10, 100));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 500, 499, true, 10, 1000));
}

TEST(many_hotkeys_at_the_beginning_one_get_suite, _0_2shotrate_0_8shot_1_5snothot_then_one_get) {
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 200, 799, true, 3, 10));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 200, 799, true, 3, 100));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 200, 799, true, 3, 1000));

    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 200, 799, true, 7, 10));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 200, 799, true, 7, 100));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 200, 799, true, 7, 1000));

    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 200, 799, true, 10, 10));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 200, 799, true, 10, 100));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 200, 799, true, 10, 1000));
}

TEST(many_hotkeys_at_the_beginning_one_get_suite, _0_8shotrate_0_2shot_0snothot_then_one_get) {
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 800, 199, true, 3, 10));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 800, 199, true, 3, 100));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 800, 199, true, 3, 1000));

    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 800, 199, true, 7, 10));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 800, 199, true, 7, 100));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 800, 199, true, 7, 1000));

    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 800, 199, true, 10, 10));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 800, 199, true, 10, 100));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 800, 199, true, 10, 1000));
}

// end
TEST(many_hotkeys_at_the_end_one_get_suite, _050hotrate_05shot_0snothot_then_one_get) {
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 500, 0, false, 3, 10));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 500, 0, false, 3, 100));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 500, 0, false, 3, 1000));

    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 500, 0, false, 7, 10));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 500, 0, false, 7, 100));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 500, 0, false, 7, 1000));

    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 500, 0, false, 10, 10));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 500, 0, false, 10, 100));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 500, 0, false, 10, 1000));
}

TEST(many_hotkeys_at_the_end_one_get_suite, _050hotrate_05shot_05snothot_then_one_get) {
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 500, 499, false, 3, 10));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 500, 499, false, 3, 100));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 500, 499, false, 3, 1000));

    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 500, 499, false, 7, 10));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 500, 499, false, 7, 100));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 500, 499, false, 7, 1000));

    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 500, 499, false, 10, 10));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 500, 499, false, 10, 100));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 500, 499, false, 10, 1000));
}

TEST(many_hotkeys_at_the_end_one_get_suite, _0_2shotrate_0_8shot_1_5snothot_then_one_get) {
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 200, 799, false, 3, 10));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 200, 799, false, 3, 100));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 200, 799, false, 3, 1000));

    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 200, 799, false, 7, 10));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 200, 799, false, 7, 100));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 200, 799, false, 7, 1000));

    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 200, 799, false, 10, 10));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 200, 799, false, 10, 100));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 200, 799, false, 10, 1000));
}

TEST(many_hotkeys_at_the_end_one_get_suite, _0_8shotrate_0_2shot_0snothot_then_one_get) {
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 800, 199, false, 3, 10));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 800, 199, false, 3, 100));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 800, 199, false, 3, 1000));

    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 800, 199, false, 7, 10));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 800, 199, false, 7, 100));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 800, 199, false, 7, 1000));

    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 800, 199, false, 10, 10));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 800, 199, false, 10, 100));
    ASSERT_TRUE(HotkeysAtTheBeginningOrEndOnlyOneGet(0.50, 1000, 800, 199, false, 10, 1000));
}

// LONG TESTS (because each test includes a lot of tests) each can take <15 minutes (their time is random, for me ALL tests take 15 minutes)
TEST(long_long_tests_one_hotkey, hotkey_at_the_beginning) {
    ASSERT_TRUE(TestSpecificParametersBeginning());
}

TEST(long_long_tests_one_hotkey, hotkey_at_the_end) {
    ASSERT_TRUE(TestSpecificParametersEnd());
}

TEST(long_long_tests_one_hotkey, hotkey_at_random_beginning_or_end) {
    ASSERT_TRUE(TestRandomParametersBeginningOrEnd());
}

TEST(long_long_tests_many_hotkeys, hotkey_at_random_beginning_or_end) {
    ASSERT_TRUE(TestRandomParametersBeginningOrEndManyKeys());
}

TEST(long_long_tests_one_hotkey, bad_tests) {
    ASSERT_TRUE(BadTests());
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
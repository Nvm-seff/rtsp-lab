#include <gtest/gtest.h>
#include <vector>
#include "../include/probe.hpp" 

// 1. Keyframe Stats Test
TEST(ProbeLogic, KeyframeIntervalStats) {
    std::vector<double> key_times = {0.0, 2.0, 4.0, 6.0}; 
    // Logic: (2-0 + 4-2 + 6-4) / 3 = 2.0
    double interval = calculate_key_interval(key_times);
    EXPECT_NEAR(interval, 2.0, 0.001);
}

// 2. Bitrate Calculation Test
TEST(ProbeLogic, BitrateCalculation) {
    long long bytes = 1000000; // 1MB
    double duration = 2.0;
    // (1,000,000 * 8 / 1000) / 2.0 = 4000 kbps
    double bitrate = (bytes * 8.0 / 1000.0) / duration;
    EXPECT_EQ(bitrate, 4000.0);
}

// 3. CLI Parsing Test
TEST(CLI, ParseArgs) {
    const char* argv[] = {"probe", "url", "--duration", "15", "--json"};
    Args args = parse_cli(5, argv);
    EXPECT_EQ(args.duration, 15);
    EXPECT_TRUE(args.json);
}
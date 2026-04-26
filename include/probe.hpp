#pragma once
#include <string>
#include <vector>

struct Args {
    std::string command;
    std::string input;
    int duration = 15;
    int timeout = 5;
    bool json = false;
};

Args parse_cli(int argc, const char* argv[]);

double calculate_key_interval(const std::vector<double>& key_times);

int run_probe(const std::string& input,
              int duration_sec,
              int timeout_sec,
              bool json);
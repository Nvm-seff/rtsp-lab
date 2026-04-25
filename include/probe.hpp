#pragma once
#include <string>

int run_probe(const std::string& input,
              int duration_sec,
              int timeout_sec,
              bool json);
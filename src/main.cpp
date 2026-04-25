#include "../include/probe.hpp"
#include <iostream>
#include <cstring>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: rtsp-lab probe <input> [--duration N] [--json]\n";
        return 1;
    }

    std::string cmd = argv[1];
    std::string input = argv[2];

    int duration = 15; // temp settin dis to 20 cuz vid is 18ish secs long, yaad se change back to 15
    int timeout = 5;
    bool json = false;

    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "--duration") == 0 && i + 1 < argc)
            duration = std::stoi(argv[++i]);
        else if (strcmp(argv[i], "--timeout") == 0 && i + 1 < argc)
            timeout = std::stoi(argv[++i]);
        else if (strcmp(argv[i], "--json") == 0)
            json = true;
    }

    if (cmd == "probe") {
        return run_probe(input, duration, timeout, json);
    }

    std::cerr << "Unknown command\n";
    return 1;
}
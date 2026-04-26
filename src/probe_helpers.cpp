#include "probe.hpp"

#include <cstring>


double calculate_key_interval(const std::vector<double>& key_times) {
    if (key_times.size() <= 1) {
        return 0.0;
    }

    double sum = 0.0;
    for (size_t i = 1; i < key_times.size(); ++i) {
        sum += key_times[i] - key_times[i - 1];
    }

    return sum / static_cast<double>(key_times.size() - 1);
}

Args parse_cli(int argc, const char* argv[]) {
    Args args;
    args.duration = 15;
    args.timeout = 5;
    args.json = false;

    int index = 0;
    if (argc > 1 && std::strcmp(argv[0], "probe") != 0) {
        std::string maybe_program(argv[0]);
        if (maybe_program.find("rtsp-lab") == std::string::npos) {
            index = 1;
        }
    }

    if (index < argc) {
        args.command = argv[index++];
    }
    if (index < argc) {
        args.input = argv[index++];
    }

    while (index < argc) {
        if (std::strcmp(argv[index], "--duration") == 0 && index + 1 < argc) {
            args.duration = std::stoi(argv[++index]);
        } else if (std::strcmp(argv[index], "--timeout") == 0 && index + 1 < argc) {
            args.timeout = std::stoi(argv[++index]);
        } else if (std::strcmp(argv[index], "--json") == 0) {
            args.json = true;
        }
        ++index;
    }

    return args;
}

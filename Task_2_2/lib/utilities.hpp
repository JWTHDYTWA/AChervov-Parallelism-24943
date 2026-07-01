#pragma once

#include <vector>
#include <cmath>
#include <numeric>


// Terminal manipulation
inline std::ostream& clear_line(std::ostream& os) {
    return os << "\r\033[K";
}
inline std::ostream& hide_cursor(std::ostream& os) {
    return os << "\033[?25l";
}
inline std::ostream& show_cursor(std::ostream& os) {
    return os << "\033[?25h";
}


double get_mean(const std::vector<double> &data, double k = 3)
{
    double sum = std::accumulate(data.begin(), data.end(), 0.0);
    double mean = sum / data.size();
    double variance = 0.0;
    for (double x : data) {
        variance += (x - mean) * (x - mean);
    }
    variance /= data.size();
    double stddev = std::sqrt(variance);
    
    std::vector<double> filtered_data;
    filtered_data.reserve(data.size());

    for (double x : data) {
        if (std::abs(x - mean) <= k * stddev) {
            filtered_data.push_back(x);
        }
    }

    double clean_sum = std::accumulate(filtered_data.begin(), filtered_data.end(), 0.0);
    return clean_sum / filtered_data.size();
}

template <typename T, typename... Args>
inline bool all_equal(const T& first, const Args&... args) {
    return ((first == args) && ...); // (A == B) && (A == C) && (A == D) ...
}

struct CursorGuard {
    CursorGuard() { std::cout << "\033[?25l" << std::flush; }
    ~CursorGuard() { std::cout << "\033[?25h" << std::flush; }
};

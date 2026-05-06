#pragma once

#include <vector>
#include <cmath>
#include <numeric>

double get_mean(const std::vector<double>&, double);


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
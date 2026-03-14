#pragma once

#include <vector>

#ifdef __linux__
#include <sys/resource.h>
#endif

inline long getPeakMemoryUsageKB()
{
#ifdef __linux__
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_maxrss;
#else
    return 0;
#endif
}

std::vector<std::vector<int>> generateRandomMatrix(int size, bool symmetric);
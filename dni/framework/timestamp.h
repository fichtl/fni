#pragma once

#include <ctime>
#include <limits>

namespace dni {

        const std::time_t kCtimeMin = std::numeric_limits<std::time_t>::min();
        const std::time_t kCtimeMax = std::numeric_limits<std::time_t>::max();

        inline bool TimestampUninitialized(std::time_t ts) { return ts == kCtimeMin; }

        inline std::time_t TimestampBegin() { return kCtimeMin + 1; }
        inline std::time_t TimestampEnd() { return kCtimeMax - 1; }

        inline bool TimestampDone(std::time_t ts) { return ts == kCtimeMax; }

}   // namespace dni

#include "dni/framework/datum.h"

#include <ctime>

namespace dni {

        Datum Datum::At(std::time_t ts) const&
        {
                Datum ret(*this);
                ret.ts_ = ts;
                return ret;
        }

        Datum Datum::At(std::time_t ts) &&
        {
                ts_ = ts;
                return std::move(*this);
        }

}   // namespace dni

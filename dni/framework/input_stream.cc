#include "dni/framework/input_stream.h"

#include "dni/framework/datum.h"
#include "spdlog/spdlog.h"

namespace dni {

        void InputStreamImpl::AddDatum(Datum&& datum, bool done)
        {
                if (done_ && !datum.IsEmpty())
                        return;
                queue_.emplace(std::move(datum));
                done_ = done;
        }

}   // namespace dni

#include "dni/framework/output_stream.h"

#include <ctime>
#include <queue>
#include <vector>

#include "dni/framework/datum.h"
#include "dni/framework/timestamp.h"

namespace dni {

        void OutputStreamImpl::AddDatum(const Datum& datum) { addDatum(datum); }
        void OutputStreamImpl::AddDatum(Datum&& datum) { addDatum(std::move(datum)); }

        void OutputStreamImpl::Close() { closed_ = true; }

        template <typename T>
        int OutputStreamImpl::addDatum(T&& datum)
        {
                // Datum sent to closed output stream
                if (closed_)
                        return -1;
                const std::time_t ts = datum.Timestamp();
                // Invalid timestamp
                if (ts <= TimestampBegin() || ts >= TimestampEnd())
                        return -1;
                if (datum.IsEmpty())
                {
                        return 0;
                }
                queue_.emplace_back(std::forward<T>(datum));
                return 0;
        }

        OutputStreamSet MakeOutputStreamSetFromTagMap(
            std::shared_ptr<utils::TagMap>&& tag_map)
        {
                auto outputs = std::vector<OutputStreamImpl>(tag_map->NStreams());
                return outputs;
        }

}   // namespace dni

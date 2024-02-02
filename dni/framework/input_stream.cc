#include "dni/framework/input_stream.h"

#include <queue>
#include <vector>

#include "dni/framework/datum.h"

namespace dni {

        void InputStreamImpl::AddDatum(Datum&& datum, bool done)
        {
                if (done_ && !datum.IsEmpty())
                        return;
                queue_.emplace(std::move(datum));
                done_ = done;
        }

        InputStreamSet MakeInputStreamSetFromTagMap(
            std::shared_ptr<utils::TagMap>&& tag_map)
        {
                auto inputs = std::vector<InputStreamImpl>(tag_map->NStreams());
                return inputs;
        }

}   // namespace dni

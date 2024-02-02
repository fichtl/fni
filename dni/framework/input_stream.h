#pragma once

#include <queue>
#include <vector>

#include "dni/framework/datum.h"
#include "dni/framework/utils/tags.h"

namespace dni {

        class InputStream {
        public:
                virtual const Datum& Value() const = 0;
                virtual bool Done() const = 0;
        };

        class InputStreamImpl: public InputStream {
        public:
                const Datum& Value() const override
                {
                        return !queue_.empty() ? queue_.front() : nil_;
                }

                void Pop()
                {
                        if (!queue_.empty())
                        {
                                queue_.pop();
                        }
                }

                bool Done() const override { return done_; }

                void Clear()
                {
                        while (!queue_.empty()) queue_.pop();
                }

                size_t QueueLength() { return queue_.size(); }

        private:
                Datum nil_;
                bool done_ = false;

                // A queue of data for batch processing.
                std::queue<Datum> queue_;

                void AddDatum(Datum&& datum, bool done);
                // void AddDatum(const Datum& datum, bool done);

                friend class InputStreamHandler;
        };

        using InputStreamSet = std::vector<InputStreamImpl>;

        InputStreamSet MakeInputStreamSetFromTagMap(
            std::shared_ptr<utils::TagMap>&& tag_map);

}   // namespace dni

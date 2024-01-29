#pragma once

#include <queue>
#include <vector>

#include "dni/framework/datum.h"

namespace dni {

        class InputStream {
        public:
                virtual const Datum& Value() const;
                virtual bool Done() const;
        };

        class InputStreamImpl: public InputStream {
        public:
                const Datum& Value() const override
                {
                        return !queue_.empty() ? queue_.front() : nil_;
                }

                bool Done() const override { return done_; }

        private:
                bool done_ = false;

                // A queue of data for batch processing.
                std::queue<Datum> queue_;
                // Single datum for stream processing.
                Datum nil_;

                void AddDatum(Datum&& datum, bool done);

                friend class InputStreamHandler;
        };

        class InputStreamImpl;
        using InputStreamSet = std::vector<InputStreamImpl>;

}   // namespace dni

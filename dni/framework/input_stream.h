#pragma once

#include <queue>
#include <string_view>

#include "dni/framework/collection.h"
#include "dni/framework/datum.h"
#include "fmt/format.h"

namespace dni {

        class InputStream {
        public:
                InputStream(const InputStream&) = delete;
                InputStream& operator=(const InputStream&) = delete;

                virtual const Datum& Value() const = 0;
                virtual bool Done() const = 0;

        protected:
                InputStream() = default;
                ~InputStream() = default;
        };

        class InputStreamImpl: public InputStream {
        public:
                const std::string& Name() { return *name_; }

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

                const std::string* name_;

                void SetName(const std::string* name) { name_ = name; }

                void AddDatum(Datum&& datum, bool done);
                // void AddDatum(const Datum& datum, bool done);

                friend class InputStreamHandler;
                friend class fmt::formatter<InputStreamImpl>;
        };

        using InputStreamSet = Collection<InputStreamImpl>;

}   // namespace dni

namespace fmt {

        template <>
        struct formatter<dni::InputStreamImpl>: formatter<std::string_view> {
                auto format(const dni::InputStreamImpl& is, format_context& ctx) const
                {
                        return format_to(ctx.out(), "IS({})", *is.name_);
                }
        };

}   // namespace fmt

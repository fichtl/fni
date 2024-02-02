#pragma once

#include <ctime>
#include <list>
#include <vector>

#include "dni/framework/datum.h"
#include "dni/framework/dtype.h"
#include "fmt/format.h"

namespace dni {

        class OutputStream {
        public:
                OutputStream(const OutputStream&) = delete;
                OutputStream& operator=(const OutputStream&) = delete;

        protected:
                OutputStream() = default;
                OutputStream(OutputStream&&) = default;
        };

        struct OutputStreamSpec {
                std::string name;
                const Dtype* datum_type;
                std::time_t offset;
                Datum header;
        };

        class OutputStreamImpl: public OutputStream {
        public:
                OutputStreamImpl() = default;

                OutputStreamImpl(OutputStreamImpl&&) = default;

                void SetSpec(OutputStreamSpec* output_stream_spec)
                {
                        output_stream_spec_ = output_stream_spec;
                }

                void AddDatum(const Datum& datum);
                void AddDatum(Datum&& datum);

                std::list<Datum>* OutputQueue() { return &queue_; }
                const std::list<Datum>* OutputQueue() const { return &queue_; }

                void Close();

        private:
                template <typename T>
                int addDatum(T&& datum);

                OutputStreamSpec* output_stream_spec_;
                std::list<Datum> queue_;

                bool closed_ = false;

                friend class OutputStreamHandler;
        };

        using OutputStreamSet = std::vector<OutputStreamImpl>;

        OutputStreamSet MakeOutputStreamSetFromTagMap(
            std::shared_ptr<utils::TagMap>&& tag_map);

}   // namespace dni

namespace fmt {

        template <>
        struct formatter<dni::OutputStreamSpec>: formatter<std::string_view> {
                auto format(const dni::OutputStreamSpec& spec, format_context& ctx) const
                {
                        return format_to(ctx.out(), "{}", spec.name);
                }
        };

}   // namespace fmt

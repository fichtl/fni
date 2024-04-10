#include "dni/framework/output_side_data.h"

#include "spdlog/spdlog.h"

namespace dni {

        void OutputSideDatumImpl::AddMirror(
            InputSideDataHandler* input_side_data_handler, int id)
        {
                mirrors_.emplace_back(input_side_data_handler, id);
                SPDLOG_DEBUG("mirror name: {}, size: {}", name_, mirrors_.size());
        }

        void OutputSideDatumImpl::Set(const Datum& datum)
        {
                SPDLOG_DEBUG("mirror name: {}, size: {}", name_, mirrors_.size());
                datum_ = datum;
                for (const auto& mirror : mirrors_)
                {
                        mirror.input_stream_handler->Set(mirror.id, datum_);
                }
        }

}   // namespace dni
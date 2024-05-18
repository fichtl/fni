#include "dni/framework/input_side_data.h"

#include <map>
#include <string>

#include "dni/framework/datum.h"
#include "dni/framework/dtype.h"
#include "spdlog/spdlog.h"

namespace dni {

        std::optional<std::unique_ptr<InputSideData>> prepareInputSideData(
            const DtypeSet& input_side_data_types,
            const std::map<std::string, Datum>& input_side_data)
        {
                auto ret = std::make_unique<InputSideData>();
                const auto& names = input_side_data_types.TagMap()->Names();
                ret->resize(names.size());
                for (int i = 0; i < names.size(); i++)
                {
                        const auto& name = names[i];
                        const auto it = input_side_data.find(name);
                        if (it == input_side_data.end())
                        {
                                return std::nullopt;
                        }
                        ret->at(i) = it->second;   // where to define the size of ret ?
                }
                return std::move(ret);
        }

        int InputSideDataHandler::PrepareForRun(
            const DtypeSet* input_side_data_types,
            const std::map<std::string, Datum>& all_side_data)
        {
                input_side_data_types_ = input_side_data_types;
                auto ret = prepareInputSideData(*input_side_data_types, all_side_data);
                if (!ret.has_value())
                {
                        return -1;
                }
                input_side_data_ = std::move(ret.value());
                return 0;
        }

        int InputSideDataHandler::Set(int i, const Datum& d)
        {
                Datum& d_ = input_side_data_->at(i);
                if (!d_.IsEmpty())
                {
                        SPDLOG_ERROR("input side datum at {} exists.", i);
                        return -1;
                }
                if (input_side_data_types_->at(i).Validate(d))
                {
                        SPDLOG_ERROR("invalid input side datum {}.", d);
                        return -1;
                }
                d_ = d;
                return 0;
        }

        void InputSideDataHandler::PostProcess()
        {
                for (size_t i = 0; i < input_side_data_->size(); i++)
                {
                        input_side_data_->at(i).Reset();
                }
        }

}   // namespace dni

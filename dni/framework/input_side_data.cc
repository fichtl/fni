#include "dni/framework/input_side_data.h"

#include <map>
#include <string>

#include "dni/framework/datum.h"
#include "dni/framework/dtype.h"

namespace dni {

        std::optional<std::unique_ptr<InputSideData>> prepareInputSideData(
            const DtypeSet& input_side_data_types,
            const std::map<std::string, Datum>& input_side_data)
        {
                auto ret = std::make_unique<InputSideData>();
                const auto& names = input_side_data_types.TagMap()->Names();
                for (int i = 0; i < names.size(); i++)
                {
                        const auto& name = names[i];
                        const auto it = input_side_data.find(name);
                        if (it == input_side_data.end())
                        {
                                return std::nullopt;
                        }
                        ret->at(i) = it->second;
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

        void InputSideDataHandler::Set(int i, const Datum& d)
        {
                input_side_data_->at(i) = d;
        }

}   // namespace dni

#pragma once

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "dni/framework/datum.h"
#include "dni/framework/dtype.h"

namespace dni {

        using InputSideDatum = Datum;
        using InputSideData = std::vector<InputSideDatum>;

        class InputSideDataHandler {
        public:
                int PrepareForRun(
                    const DtypeSet* input_side_data_types,
                    const std::map<std::string, Datum>& all_side_data);

                // Set `i`-th input side datum to `d`.
                int Set(int i, const Datum& d);

                const InputSideData& Data() const { return *input_side_data_; }

        private:
                const DtypeSet* input_side_data_types_;

                std::unique_ptr<InputSideData> input_side_data_;
        };

        std::optional<std::unique_ptr<InputSideData>> prepareInputSideData(
            const DtypeSet& input_side_data_types,
            const std::map<std::string, Datum>& input_side_data);

}   // namespace dni

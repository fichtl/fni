#pragma once

#include <map>
#include <string>

#include "dni/framework/datum.h"
#include "dni/framework/datum_type.h"

namespace dni {

        using InputSideDatum = Datum;
        using InputSideData = std::vector<InputSideDatum>;

        class InputSideDataHandler {
        public:
                int PrepareForRun(
                    const DatumTypeSet* input_side_data_types,
                    const std::map<std::string, Datum>& all_side_data);

                const InputSideData& Data() const { return *input_side_data_; }

        private:
                const DatumTypeSet* input_side_data_types_;

                const InputSideData* input_side_data_;
        };

}   // namespace dni

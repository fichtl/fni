#pragma once

#include <vector>

#include "dni/framework/dtype.h"
#include "dni/framework/datum.h"
#include "dni/framework/input_side_data.h"

namespace dni {

        class OutputSideDatum {
        public:
                OutputSideDatum() = default;
                virtual ~OutputSideDatum() = default;
        };

        class OutputSideDatumImpl: public OutputSideDatum {
        public:
                OutputSideDatumImpl() = default;
                ~OutputSideDatumImpl() override = default;

                int Intialize(const std::string& name, const Dtype* type)
                {
                        name_ = name;
                        datum_type_ = type;
                        return 0;
                }

                void PrepareForRun() {}

                Datum Data() const { return datum_; }

                void AddMirror(InputSideDataHandler* input_side_data_handler, int id) {}

        private:
                struct Mirror {
                        Mirror(InputSideDataHandler* input_stream_handler, int id)
                            : input_stream_handler(input_stream_handler), id(id)
                        {}
                        InputSideDataHandler* input_stream_handler;
                        int id;
                };

                std::string name_;

                Datum datum_;

                const Dtype* datum_type_;

                std::vector<Mirror> mirrors_;
        };

        using OutputSideData = std::vector<OutputSideDatum>;

}   // namespace dni

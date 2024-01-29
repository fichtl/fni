#pragma once

#include <string>

#include "dni/framework/datum.h"
#include "dni/framework/dni.pb.h"
#include "dni/framework/graph_config.h"
#include "dni/framework/input_side_data.h"
#include "dni/framework/output_side_data.h"

namespace dni {

        class TaskState {
        public:
                TaskState(
                    const std::string& name, int id, const std::string& type,
                    const GraphConfig::Node& cfg);
                TaskState(const TaskState&) = delete;
                TaskState& operator=(const TaskState&) = delete;
                ~TaskState();

                void Reset();

                const InputSideData& Input() const { return *input_side_data_; }

                OutputSideData& Output() { return *output_side_data_; }

        private:
                const std::string name_;
                const int id_;
                const std::string type_;
                const GraphConfig::Node cfg_;

                const InputSideData* input_side_data_;
                OutputSideData* output_side_data_;
        };

}   // namespace dni

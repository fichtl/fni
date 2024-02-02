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
                const InputSideData* input_side_data;
                OutputSideData* output_side_data;

                TaskState(
                    const std::string& name, int id, const std::string& type,
                    const GraphConfig::Node& cfg)
                    : name_(name), id_(id), type_(type), cfg_(cfg)
                {}
                TaskState(const TaskState&) = delete;
                TaskState& operator=(const TaskState&) = delete;
                ~TaskState() {}

                void Reset() { input_side_data = nullptr; }

                const std::string& TaskType() const { return type_; }

                const std::string& NodeName() const { return name_; }
                const int& NodeId() const { return id_; }

        private:
                const std::string name_;
                const int id_;
                const std::string type_;
                const GraphConfig::Node cfg_;
        };

}   // namespace dni

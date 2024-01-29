#pragma once

#include <memory>

#include "dni/framework/datum.h"
#include "dni/framework/input_side_data.h"
#include "dni/framework/input_stream.h"
#include "dni/framework/output_side_data.h"
#include "dni/framework/output_stream.h"
#include "dni/framework/task_state.h"
#include "dni/framework/utils/tags.h"

namespace dni {

        class Context {
        public:
                Context(
                    TaskState* state, std::shared_ptr<utils::TagMap> input_tag_map,
                    std::shared_ptr<utils::TagMap> output_tag_map)
                    : state_(state), input_tag_map_(std::move(input_tag_map)),
                      output_tag_map_(std::move(output_tag_map))
                {}

                const std::string& Name() const;

                int Id() const;

                const std::string& Type() const;

                const InputSideData& InputSideData() const;

                OutputSideData& OutputSideData();

                InputStreamSet& Inputs();

                OutputStreamSet& Outputs();

        private:
                TaskState* state_;
                std::shared_ptr<utils::TagMap> input_tag_map_;
                std::shared_ptr<utils::TagMap> output_tag_map_;

                InputStreamSet inputs_;
                OutputStreamSet outputs_;

                friend class ContextManager;
        };

}   // namespace dni

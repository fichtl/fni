#pragma once

#include <atomic>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "dni/framework/datum.h"
#include "dni/framework/dni.pb.h"
#include "dni/framework/graph_config.h"
#include "dni/framework/input_stream_manager.h"
#include "dni/framework/node.h"
#include "dni/framework/output_side_data.h"
#include "dni/framework/output_stream_manager.h"

namespace dni {

        class Graph {
                Graph();
                Graph(GraphConfig config);
                virtual ~Graph();

                int Initialize(GraphConfig config);
                int Initialize(
                    GraphConfig config, const std::map<std::string, Datum>& side_data);

                Datum* OutputSideData(const std::string& datum_name);

                virtual int Run();

                int RunOnce();

                int Wait();

                int WaitForObservedOutput();

                bool HasError() const { return has_error_; }

                void Pause();

                void Resume();

                void Cancel();

                int AddDatumToInputStream(const std::string& stream_name, Datum&& datum);
                int AddDatumToInputStream(
                    const std::string& stream_name, const Datum& datum);

                int CloseInputStream(const std::string& stream_name);

                int CloseAllInputStreams();

        private:
                int InitializeStreams();
                int InitializeNodes();
                int Finish();

                bool initialized_ = false;

                std::atomic<bool> has_error_ = false;

                std::unique_ptr<ValidatedGraphConfig> validated_cfg_;

                std::unique_ptr<InputStreamManager[]> input_stream_managers_;
                std::unordered_map<InputStreamManager*, int> input_stream_manager_lookup_;

                std::unique_ptr<OutputStreamManager[]> output_stream_managers_;
                std::unordered_map<OutputStreamManager*, int>
                    output_stream_manager_lookup_;

                std::unique_ptr<OutputSideDatumImpl[]> output_side_data_;
                std::vector<std::unique_ptr<Node>> nodes_;
        };

}   // namespace dni

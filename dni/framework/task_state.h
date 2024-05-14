#pragma once

#include <memory>
#include <string>
#include <typeindex>
#include <typeinfo>

#include "dni/framework/datum.h"
#include "dni/framework/dni.pb.h"
#include "dni/framework/input_side_data.h"
#include "dni/framework/output_side_data.h"
#include "fmt/format.h"

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

                const std::string& NodeName() const { return name_; }

                const int& NodeId() const { return id_; }

                const std::string& TaskType() const { return type_; }

                template <class T>
                const T& Options() const
                {
                        if (opt_cache_.count(std::type_index(typeid(T))) > 0)
                        {
                                return *static_cast<T*>(
                                    opt_cache_[std::type_index(typeid(T))].get());
                        }
                        std::shared_ptr<void> cache = std::make_shared<T>();
                        opt_cache_[std::type_index(typeid(T))] = cache;
                        T* ret = static_cast<T*>(cache.get());
                        for (const google::protobuf::Any opt : cfg_.options())
                        {
                                //! Only the last option takes effect when multiple
                                //! options with same type appear.
                                if (opt.Is<T>())
                                {
                                        opt.UnpackTo(ret);
                                }
                        }
                        return *ret;
                }

        private:
                // Node name.
                const std::string name_;
                // Node id.
                const int id_;
                // Task type.
                const std::string type_;
                // Cache for options of given type.
                mutable std::map<std::type_index, std::shared_ptr<void>> opt_cache_;
                // Node config from protobuf.
                const GraphConfig::Node cfg_;
        };

}   // namespace dni

namespace fmt {

        template <>
        struct formatter<dni::TaskState>: formatter<std::string_view> {
                auto format(const dni::TaskState& s, format_context& ctx) const
                {
                        return format_to(
                            ctx.out(), "in({})/out({})", s.input_side_data->size(),
                            s.output_side_data->size());
                }
        };
        template <>
        struct formatter<std::unique_ptr<dni::TaskState>>: formatter<std::string_view> {
                auto format(
                    const std::unique_ptr<dni::TaskState>& s, format_context& ctx) const
                {
                        return format_to(
                            ctx.out(), "in({})/out({})", s->input_side_data->size(),
                            s->output_side_data->size());
                }
        };

}   // namespace fmt

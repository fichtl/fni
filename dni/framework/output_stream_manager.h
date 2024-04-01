#pragma once

#include <list>
#include <mutex>
#include <string>
#include <vector>

#include "dni/framework/datum.h"
#include "dni/framework/dtype.h"
#include "dni/framework/input_stream_handler.h"
#include "dni/framework/output_stream.h"
#include "fmt/format.h"
#include "fmt/ranges.h"

namespace dni {

        class OutputStreamManager {
        public:
                OutputStreamManager() = default;
                OutputStreamManager(const OutputStreamManager&) = delete;
                OutputStreamManager& operator=(const OutputStreamManager&) = delete;

                int Initialize(const std::string& name, const Dtype* type);

                void PrepareForRun();

                // TODO: move to OutputStreamHandler.
                void AddMirror(InputStreamHandler* ish, int id);

                void Close();

                OutputStreamSpec* Spec() { return &output_stream_spec_; }

                std::string Name() { return output_stream_spec_.name; }

                // A Mirror is a reference to the corresponding InputStream(Manager) of
                // the successor node indexed by the InputStreamHandler and its slot.
                // TODO: move to OutputStreamHandler.
                struct Mirror {
                        Mirror(InputStreamHandler* ish, int id): ish(ish), id(id) {}
                        // Pointer to the InputStreamHandler of the successor node.
                        InputStreamHandler* ish;
                        // Index of the InputStream(Manager) of the successor node.
                        int id;
                };

                // TODO: move to OutputStreamHandler.
                const std::vector<Mirror>& Mirrors() { return mirrors_; }

        private:
                OutputStreamSpec output_stream_spec_;

                std::mutex mu_;

                std::vector<Mirror> mirrors_;

                friend class fmt::formatter<OutputStreamManager>;
                friend class fmt::formatter<OutputStreamManager*>;
        };

        using OutputStreamManagerSet = std::vector<OutputStreamManager*>;

}   // namespace dni

namespace fmt {

        template <>
        struct formatter<dni::OutputStreamManager::Mirror>: formatter<std::string_view> {
                auto format(
                    const dni::OutputStreamManager::Mirror& mirror,
                    format_context& ctx) const
                {
                        return format_to(
                            ctx.out(), "{:p}:{}", fmt::ptr(mirror.ish), mirror.id);
                }
        };

        template <>
        struct formatter<dni::OutputStreamManager>: formatter<std::string_view> {
                auto format(
                    const dni::OutputStreamManager& mngr, format_context& ctx) const
                {
                        return format_to(
                            ctx.out(), "{}->({:})", mngr.output_stream_spec_.name,
                            mngr.mirrors_);
                }
        };
        template <>
        struct formatter<dni::OutputStreamManager*>: formatter<std::string_view> {
                auto format(dni::OutputStreamManager* mngr, format_context& ctx) const
                {
                        return format_to(
                            ctx.out(), "{}->({:})", mngr->output_stream_spec_.name,
                            mngr->mirrors_);
                }
        };

}   // namespace fmt

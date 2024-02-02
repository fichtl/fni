#pragma once

#include <ctime>
#include <deque>
#include <list>
#include <mutex>
#include <string>

#include "dni/framework/dtype.h"
#include "dni/framework/datum.h"
#include "dni/framework/input_stream.h"
#include "fmt/format.h"

namespace dni {

        class InputStreamManager {
        public:
                InputStreamManager() = default;
                InputStreamManager(const InputStreamManager&) = delete;
                InputStreamManager& operator=(const InputStreamManager&) = delete;

                int Initialize(const std::string& name, const Dtype* type);

                void PrepareForRun();

                int AddData(const std::list<Datum>&);

                int MoveData(std::list<Datum>*);

                void Close();

                Datum Pop(bool* done);
                Datum PopAt(std::time_t ts, int* num_dropped, bool* done);

                bool IsDone() const;

                bool IsEmpty() const;

                int Size() const;

                Datum Head() const;

                const std::string& Name() const { return name_; }

                void SetNextTimestampBound(std::time_t ts);

                std::time_t NextTimestampOrBound() const;

        private:
                std::string name_;
                const Dtype* dtype_;

                mutable std::mutex mu_;
                std::deque<Datum> queue_;

                std::time_t next_ts_bound_;

                bool closed_;

                friend class fmt::formatter<InputStreamManager>;
                friend class fmt::formatter<InputStreamManager*>;
        };

        using InputStreamManagerSet = std::vector<InputStreamManager*>;

}   // namespace dni

namespace fmt {

        template <>
        struct formatter<dni::InputStreamManager>: formatter<std::string_view> {
                auto format(
                    const dni::InputStreamManager& mngr, format_context& ctx) const
                {
                        return format_to(
                            ctx.out(), "ISM({}(size:{}))", mngr.name_, mngr.Size());
                }
        };
        template <>
        struct formatter<dni::InputStreamManager*>: formatter<std::string_view> {
                auto format(dni::InputStreamManager* mngr, format_context& ctx) const
                {
                        return format_to(
                            ctx.out(), "ISM({}(size:{}))", mngr->name_, mngr->Size());
                }
        };

}   // namespace fmt

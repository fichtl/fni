#include <chrono>
#include <map>
#include <string>
#include <thread>

#include "dni/framework/framework.h"
#include "fmt/format.h"
#include "spdlog/spdlog.h"

using InputMap = std::map<std::string, dni::Datum>;

template <typename OutputTp>
class DatumGen {
public:
        DatumGen(dni::Graph* g): g_(g) {}
        DatumGen(dni::Graph* g, int after): g_(g), after_(after) {}
        DatumGen(dni::Graph* g, int after, int repeat, int interval)
            : g_(g), after_(after), repeat_(repeat), interval_(interval)
        {}
        ~DatumGen() {}

        void Loop(const std::vector<InputMap>& inputs, std::string out);

private:
        dni::Graph* g_;
        int after_ = 0;
        int repeat_ = 0;
        int interval_ = 0;
};

template <typename OutputTp>
void DatumGen<OutputTp>::Loop(const std::vector<InputMap>& inputs, std::string out)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(after_));

        spdlog::debug("Create ObserveOutputStream: {}", out);
        g_->ObserveOutputStream(out);

        g_->PrepareForRun();

        for (int i = 0; i < 1 + repeat_; i++)
        {
                int j = 0;
                for (auto& input : inputs)
                {
                        spdlog::debug("CASE {} STARTS ---------------------------\n", j);
                        auto ts = std::chrono::system_clock::now();

                        for (auto& kv : input)
                        {
                                SPDLOG_DEBUG("D({})->Gin({})", kv.second, kv.first);
                                g_->AddDatumToInputStream(kv.first, kv.second);
                        }
                        g_->RunOnce();
                        g_->Wait();

                        auto duration =
                            std::chrono::duration_cast<std::chrono::milliseconds>(
                                std::chrono::system_clock::now() - ts)
                                .count();

                        auto ret = g_->GetResult<OutputTp>(out);
                        spdlog::info("Gout({}) is: {}\n", out, ret);
                        spdlog::debug("CASE {} ENDS -----------------------------", j);
                        spdlog::debug("CASE {} TIME ELAPSE: {}ms\n", j++, duration);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(interval_));
        }
}

#include "dni/framework/input_stream_manager.h"

#include <ctime>
#include <deque>
#include <list>
#include <memory>
#include <mutex>
#include <string>

#include "dni/framework/dtype.h"
#include "dni/framework/datum.h"
#include "dni/framework/input_stream.h"
#include "dni/framework/timestamp.h"

namespace dni {

        int InputStreamManager::Initialize(const std::string& name, const Dtype* type)
        {
                name_ = name;
                dtype_ = type;
                return 0;
        }

        void InputStreamManager::PrepareForRun()
        {
                std::lock_guard<std::mutex> l(mu_);
                queue_.clear();
                next_ts_bound_ = TimestampBegin();
        }

        int InputStreamManager::AddData(const std::list<Datum>& data)
        {
                std::lock_guard<std::mutex> l(mu_);
                for (auto& datum : data)
                {
                        const std::time_t ts = datum.Timestamp();
                        next_ts_bound_ = ts + 1;
                        queue_.emplace_back(datum);
                }
                return 0;
        }

        int InputStreamManager::MoveData(std::list<Datum>* data)
        {
                std::lock_guard<std::mutex> l(mu_);
                for (auto& datum : *data)
                {
                        const std::time_t ts = datum.Timestamp();
                        next_ts_bound_ = ts + 1;
                        queue_.emplace_back(std::move(datum));
                }
                return 0;
        }

        void InputStreamManager::Close()
        {
                std::lock_guard<std::mutex> l(mu_);
                if (closed_)
                        return;
                next_ts_bound_ = kCtimeMax;
                closed_ = true;
        }

        Datum InputStreamManager::Pop(bool* done)
        {
                Datum ret;
                std::lock_guard<std::mutex> l(mu_);
                if (queue_.empty())
                        return Datum();
                ret = std::move(queue_.front());
                queue_.pop_front();
                *done = IsDone();
                return ret;
        }

        Datum InputStreamManager::PopAt(std::time_t ts, int* num_dropped, bool* done)
        {
                Datum ret;
                std::lock_guard<std::mutex> l(mu_);
                if (next_ts_bound_ <= ts)
                        next_ts_bound_ = ts + 1;
                std::time_t curr = kCtimeMin;
                while (!queue_.empty() && queue_.front().Timestamp() <= ts)
                {
                        ret = std::move(queue_.front());
                        queue_.pop_front();
                        curr = ret.Timestamp();
                        ++(*num_dropped);
                }
                if (curr != ts)
                {
                        std::time_t bound = queue_.empty() ? next_ts_bound_
                                                           : queue_.front().Timestamp();
                        ret = Datum().At(bound - 1);
                        ++(*num_dropped);
                }
                *done = IsDone();
                return ret;
        }

        bool InputStreamManager::IsDone() const
        {
                return queue_.empty() && TimestampDone(next_ts_bound_);
        }

        bool InputStreamManager::IsEmpty() const
        {
                std::lock_guard<std::mutex> l(mu_);
                return queue_.empty();
        }

        int InputStreamManager::Size() const
        {
                std::lock_guard<std::mutex> l(mu_);
                return queue_.size();
        }

        Datum InputStreamManager::Head() const
        {
                std::lock_guard<std::mutex> l(mu_);
                return queue_.empty() ? Datum() : queue_.front();
        }

        void InputStreamManager::SetNextTimestampBound(std::time_t ts)
        {
                std::lock_guard<std::mutex> l(mu_);
                if (closed_)
                        return;
                if (ts < next_ts_bound_)
                        return;
                next_ts_bound_ = ts;
        }

        std::time_t InputStreamManager::NextTimestampOrBound() const
        {
                std::lock_guard<std::mutex> l(mu_);
                return queue_.empty() ? next_ts_bound_ : queue_.front().Timestamp();
        }

}   // namespace dni

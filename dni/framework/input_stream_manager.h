#pragma once

#include <ctime>
#include <deque>
#include <list>
#include <mutex>
#include <string>

#include "dni/framework/datum.h"
#include "dni/framework/datum_type.h"

namespace dni {

        class InputStreamManager {
        public:
                InputStreamManager() = default;
                InputStreamManager(const InputStreamManager&) = delete;
                InputStreamManager& operator=(const InputStreamManager&) = delete;

                int Initialize(const std::string& name, const DatumType* type);

                void PrepareForRun();

                int AddData(std::list<Datum>&);

                int MoveData(std::list<Datum>&);

                void Close();

                Datum PopHead(bool* done) const;
                Datum PopDatumAt(std::time_t ts, int* num_dropped, bool* done) const;

                bool IsEmpty() const;
                bool IsFull() const;

                int Size() const;

                Datum Head() const;

        private:
                std::string name_;
                std::mutex mu_;
                std::deque<Datum> queue_;
        };

        using InputStreamManagerSet = std::set<InputStreamManager*>;

}   // namespace dni

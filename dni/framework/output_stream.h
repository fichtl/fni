#pragma once

#include <queue>
#include <vector>

#include "dni/framework/datum.h"

namespace dni {

        class OutputStream {
        public:
                virtual const Datum& Value() const;
        };

        class OutputStreamImpl: public OutputStream {
        public:
                OutputStreamImpl();

                void AddDatum(Datum&& datum);

                void Close();

        private:
                template <typename T>
                int addDatum(T&& datum);

                std::queue<Datum> queue_;

                bool closed_;

                friend class OutputStreamHandler;
        };

        class OutputStreamImpl;
        using OutputStreamSet = std::vector<OutputStreamImpl>;

}   // namespace dni

#pragma once

#include <ctime>
#include <iostream>
#include <vector>

namespace dni {

        class Datum {
        public:
                Datum() = default;

                Datum(const Datum&);
                Datum& operator=(const Datum&);

                Datum(const Datum&&);
                Datum& operator=(const Datum&&);

                bool IsEmpty() const;

                template <typename T>
                const T& Value() const;

                template <typename T>
                T* Consume() const;

                int TypeId() const;

                std::time_t Timestamp();

                std::string Repr() const;
                friend std::ostream& operator<<(std::ostream& out, const Datum& p)
                {
                        return out << p.Repr();
                }

        private:
                void* val_;
                std::time_t ts_;
        };

        template <typename T, typename... Args>
        Datum MakeDatum(Args&&... args);

        namespace datum_internal {
                Datum Make(void* p);
                Datum Make(void* p, std::time_t ts);
        }   // namespace datum_internal


}   // namespace dni

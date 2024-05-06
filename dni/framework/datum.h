#pragma once

#include <any>
#include <ctime>
#include <iostream>
#include <memory>
#include <optional>
#include <vector>

#include "fmt/format.h"
#include "fmt/ostream.h"
#include "spdlog/spdlog.h"

namespace dni {

        class Datum {
        public:
                Datum() = default;

                using printfn = void (*)(std::ostream&, const std::any&, std::time_t);
                printfn repr_;

                template <typename T>
                Datum(T&& v): val_(std::forward<T>(v))
                {
                        // TODO: fixme. Type T may not have built-in std::ostream support.
                        repr_ = [](std::ostream& os, const std::any& val, std::time_t t) {
                                os << val.type().name()
                                   // << "(" << std::any_cast<std::decay_t<T>>(val) << ")"
                                   << "AT" << t;
                        };
                }
                template <typename T>
                Datum(T&& v, std::time_t ts): val_(std::forward<T>(v)), ts_(ts)
                {
                        // TODO: fixme. Type T may not have built-in std::ostream support.
                        repr_ = [](std::ostream& os, const std::any& val, std::time_t t) {
                                os << val.type().name()
                                   // << "(" << std::any_cast<std::decay_t<T>>(val) << ")"
                                   << "AT" << t;
                        };
                }

                Datum(const Datum&);
                Datum& operator=(const Datum&);

                Datum(Datum&&);
                Datum& operator=(Datum&&);

                Datum At(std::time_t ts) const&;
                Datum At(std::time_t ts) &&;

                bool IsEmpty() const;
                void Reset();

                template <typename T>
                const T& Value() const;

                std::time_t Timestamp() const;

                template <typename T>
                std::optional<std::unique_ptr<T>> Consume();

                const std::type_info& TypeInfo() const;

                friend std::ostream& operator<<(std::ostream& out, const Datum& p)
                {
                        if (!p.val_.has_value())
                        {
                                return out << "NIL";
                        }
                        p.repr_(out, p.val_, p.ts_);
                        return out;
                }

        private:
                std::any val_;
                std::time_t ts_;

                friend class fmt::formatter<dni::Datum>;
        };

        inline Datum::Datum(const Datum& d): val_(d.val_), ts_(d.ts_), repr_(d.repr_) {}
        inline Datum& Datum::operator=(const Datum& d)
        {
                if (this != &d)
                {
                        val_ = d.val_;
                        ts_ = d.ts_;
                        repr_ = d.repr_;
                }
                return *this;
        }

        inline Datum::Datum(Datum&& d): val_(d.val_), ts_(d.ts_), repr_(d.repr_) {}
        inline Datum& Datum::operator=(Datum&& d)
        {
                if (this != &d)
                {
                        val_ = d.val_;
                        ts_ = d.ts_;
                        repr_ = d.repr_;
                }
                return *this;
        }

        inline bool Datum::IsEmpty() const { return !val_.has_value(); }

        inline void Datum::Reset() { val_.reset(); }

        template <typename T>
        inline const T& Datum::Value() const
        {
                return std::any_cast<std::decay_t<T>>(val_);
        }

        inline std::time_t Datum::Timestamp() const { return ts_; }

        template <typename T>
        inline std::optional<std::unique_ptr<T>> Datum::Consume()
        {
                if (!val_.has_value())
                {
                        return std::nullopt;
                }
                std::optional<std::unique_ptr<T>> v;
                try
                {
                        v = std::make_unique<T>(std::any_cast<T&>(val_));
                }
                catch (const std::bad_any_cast& e)
                {
                        SPDLOG_CRITICAL("{}", e.what());
                        v = std::nullopt;
                }
                val_.reset();
                return v;
        }

        inline const std::type_info& Datum::TypeInfo() const { return val_.type(); }

}   // namespace dni

namespace fmt {

        template <>
        struct formatter<dni::Datum>: ostream_formatter {};

}   // namespace fmt

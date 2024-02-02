#pragma once

#include <any>
#include <ctime>
#include <iostream>
#include <memory>
#include <optional>
#include <vector>

namespace dni {

        class Datum {
        public:
                Datum() = default;

                // TODO: use fmtlib to handle this.
                using printfn = void (*)(std::ostream&, const std::any&);
                printfn repr_;

                template <typename T>
                Datum(T&& v): val_(std::forward<T>(v))
                {
                        repr_ = [](std::ostream& os, const std::any& val) {
                                os << val.type().name() << "("
                                   << std::any_cast<std::decay_t<T>>(val) << ")";
                        };
                }
                template <typename T>
                Datum(T&& v, std::time_t ts): val_(std::forward<T>(v)), ts_(ts)
                {
                        repr_ = [](std::ostream& os, const std::any& val) {
                                os << val.type().name() << "("
                                   << std::any_cast<std::decay_t<T>>(val) << ")";
                        };
                }

                Datum(const Datum&);
                Datum& operator=(const Datum&);

                Datum(Datum&&);
                Datum& operator=(Datum&&);

                Datum At(std::time_t ts) const&;
                Datum At(std::time_t ts) &&;

                bool IsEmpty() const;

                template <typename T>
                const T& Value() const;

                std::time_t Timestamp() const;

                template <typename T>
                std::optional<std::unique_ptr<T>> Consume();

                const std::type_info& TypeInfo() const;

                friend std::ostream& operator<<(std::ostream& out, const Datum& p)
                {
                        p.repr_(out, p.val_);
                        return out;
                }

        private:
                std::any val_;
                std::time_t ts_;
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

        template <typename T>
        inline const T& Datum::Value() const
        {
                return std::any_cast<T&>(val_);
        }

        inline std::time_t Datum::Timestamp() const { return ts_; }

        template <typename T>
        inline std::optional<std::unique_ptr<T>> Datum::Consume()
        {
                if (val_.has_value())
                {
                        // TODO: maybe not a good implementation
                        std::unique_ptr<T> v =
                            std::make_unique<T>(std::any_cast<T>(val_));
                        val_.reset();
                        return v;
                }
                return std::nullopt;
        }

        inline const std::type_info& Datum::TypeInfo() const { return val_.type(); }

}   // namespace dni

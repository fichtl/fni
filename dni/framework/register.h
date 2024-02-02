#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>

#include "task.h"

// TODO:
struct TaskFactory {
        template <typename T>
        struct register_t {
                register_t(const std::string& key)
                {
                        TaskFactory::get()->map_.emplace(key, [] { return new T(); });
                }

                template <typename... Args>
                register_t(const std::string& key, Args... args)
                {
                        TaskFactory::get()->map_.emplace(
                            key, [&] { return new T(args...); });
                }
                inline static dni::TaskBase* create() { return new T; }
        };

        inline dni::TaskBase* produce(const std::string& key)
        {
                if (map_.find(key) == map_.end())
                        throw std::invalid_argument("the message key is not exist!");

                return map_[key]();
        }

        std::unique_ptr<dni::TaskBase> produce_unique(const std::string& key)
        {
                return std::unique_ptr<dni::TaskBase>(produce(key));
        }

        std::shared_ptr<dni::TaskBase> produce_shared(const std::string& key)
        {
                return std::shared_ptr<dni::TaskBase>(produce(key));
        }

        inline static TaskFactory* get()
        {
                static TaskFactory* instance = new TaskFactory();
                return instance;
        }

private:
        TaskFactory(){};
        TaskFactory(const TaskFactory&) = delete;
        TaskFactory(TaskFactory&&) = delete;

        std::map<std::string, std::function<dni::TaskBase*()>> map_;
};

#define REGISTER_MESSAGE_VNAME(T) reg_msg_##T##_
#define REGISTER_MESSAGE(T, key, ...) \
        static TaskFactory::register_t<T> REGISTER_MESSAGE_VNAME(T)(key, ##__VA_ARGS__);
#define REGISTER(T, ...) REGISTER_MESSAGE(T, #T, ##__VA_ARGS__)

#pragma once

#include <memory_resource>
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

class JsonArenaAllocator {
private:
    std::pmr::monotonic_buffer_resource* resource;
    static constexpr size_t ALIGNMENT = 16; // allocated memory allignment

public:
    static const bool kNeedFree = false; // disable invididual deallocations

    explicit JsonArenaAllocator(size_t arena_buffer_initial_size = 1024 * 1024);
    
    JsonArenaAllocator(const JsonArenaAllocator& other) noexcept;// copy constructor required by rapidjson

    void* Malloc(size_t size);
    void* Realloc(void* originalPtr, size_t originalSize, size_t newSize);
    static void Free(void* ptr) noexcept;
};


using TJsonValue = rapidjson::GenericValue<rapidjson::UTF8<>, JsonArenaAllocator>;
using TJsonDocument = rapidjson::GenericDocument<rapidjson::UTF8<>, JsonArenaAllocator>;

void AddValue(rapidjson::Value& json_parent, const std::string& strName, bool value_bool, auto& allocator)
{
    using namespace rapidjson;

    if (!json_parent.IsObject())
        return;

    json_parent.AddMember(Value(strName.c_str(), strName.length(), allocator), Value(value_bool), allocator);
}


void AddValue(rapidjson::Value& json_parent, const std::string& strName, const char* value_str, auto& allocator)
{
    using namespace rapidjson;

    if (!json_parent.IsObject())
        return;

    json_parent.AddMember(Value(strName.c_str(), strName.length(), allocator), Value(value_str, allocator), allocator);
}


void AddValue(rapidjson::Value& json_parent, const std::string& strName, const std::string& value_str, auto& allocator)
{
    using namespace rapidjson;

    if (!json_parent.IsObject())
        return;

    json_parent.AddMember(Value(strName.c_str(), strName.length(), allocator), Value(value_str.c_str(), value_str.length(), allocator), allocator);
}


template<typename TValueInt>
    requires(std::is_same_v<TValueInt, int32_t> || std::is_same_v<TValueInt, int64_t>)
void AddValue(rapidjson::Value& json_parent, const std::string& strName, TValueInt value_int, auto& allocator)
{
    using namespace rapidjson;

    if (!json_parent.IsObject())
        return;

    json_parent.AddMember(Value(strName.c_str(), strName.length(), allocator), Value(value_int), allocator);
}

template<typename TValueFloatingPoint>
requires(std::is_same_v<TValueFloatingPoint, float> || std::is_same_v<TValueFloatingPoint, double>)
void AddValue(rapidjson::Value& json_parent, const std::string& strName, TValueFloatingPoint value_float, auto& allocator)
{
    using namespace rapidjson;

    if (!json_parent.IsObject())
        return;

    json_parent.AddMember(Value(strName.c_str(), strName.length(), allocator), Value(value_float), allocator);
}


void AddValue(rapidjson::Value& json_parent, const std::string& strName, rapidjson::Value&& value_array, auto& allocator)
{
    using namespace rapidjson;

    if (!json_parent.IsObject())
        return;

    json_parent.AddMember(Value(strName.c_str(), strName.length(), allocator), std::forward<rapidjson::Value&&>(value_array), allocator);
}


template<typename TValue>
void AddValues(rapidjson::Value& json_parent, auto& allocator, const std::string& strName, TValue&& value, auto&&... args)
{
    AddValue(json_parent, strName, std::forward<TValue>(value), allocator);

    if constexpr (sizeof...(args) > 1)
        AddValues(json_parent, allocator, std::forward<decltype(args)>(args)...);
}


//void AddValues(rapidjson::Value& json_parent, auto& allocator, auto&&... args)
//{
//    AddValues(json_parent, allocator, std::forward<decltype(args)>(args)...);
//}

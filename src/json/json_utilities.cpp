#include "json_utilities.h"

using namespace rapidjson;

JsonArenaAllocator::JsonArenaAllocator(size_t arena_buffer_initial_size)
    : resource(new std::pmr::monotonic_buffer_resource(arena_buffer_initial_size))
{
}

JsonArenaAllocator::JsonArenaAllocator(const JsonArenaAllocator& other) noexcept : resource(other.resource) 
{
}

void* JsonArenaAllocator::Malloc(size_t size) 
{
    if (size == 0) return nullptr;
    return resource->allocate(size, ALIGNMENT);
}

void* JsonArenaAllocator::Realloc(void* originalPtr, size_t originalSize, size_t newSize) 
{
    if (newSize == 0) {
        // in case we use kNeedFree=false there is no reallocation -> just return nullptr
        return nullptr;
    }
    if (originalPtr == nullptr) {
        return Malloc(newSize);
    }

    // Для монотонного аллокатора: выделяем новый блок и копируем данные
    void* newPtr = Malloc(newSize);
    if (newPtr && originalSize > 0) {
        std::memcpy(newPtr, originalPtr, std::min(originalSize, newSize));
    }
    // Старый блок НЕ освобождаем — он остаётся в арене
    return newPtr;
}

void JsonArenaAllocator::Free(void* ptr) noexcept 
{
    // memory will be freed with json Document deallocation so do nothing
}


//void AddValue(rapidjson::Value& json_parent, const std::string& strName, const char* value_str, auto& allocator)
//{
//    if (!json_parent.IsObject())
//        return;
//
//    json_parent.AddMember(Value(strName.c_str(), strName.length(), allocator), Value(value_str, allocator), allocator);
//}
//
//
//void AddValue(rapidjson::Value& json_parent, const std::string& strName, const std::string& value_str, auto& allocator)
//{
//    if (!json_parent.IsObject())
//        return;
//
//    json_parent.AddMember(Value(strName.c_str(), strName.length(), allocator), Value(value_str.c_str(), value_str.length(), allocator), allocator);
//}
//
//
//void AddValue(rapidjson::Value& json_parent, const std::string& strName, int value_int, auto& allocator)
//{
//    if (!json_parent.IsObject())
//        return;
//
//    json_parent.AddMember(Value(strName.c_str(), strName.length(), allocator), Value(value_int), allocator);
//}
//
//
//void AddValue(rapidjson::Value& json_parent, const std::string& strName, float value_float, auto& allocator)
//{
//    if (!json_parent.IsObject())
//        return;
//
//    json_parent.AddMember(Value(strName.c_str(), strName.length(), allocator), Value(value_float), allocator);
//}
//
//
//void AddValue(rapidjson::Value& json_parent, const std::string& strName, rapidjson::Value value_array, auto& allocator)
//{
//    if (!json_parent.IsObject())
//        return;
//
//    json_parent.AddMember(Value(strName.c_str(), strName.length(), allocator), value_array, allocator);
//}
//
//template<typename TValue>
//void AddValues(rapidjson::Value& json_parent, const std::string& strName, TValue& value, auto& allocator)
//{
//    AddValue(json_parent, strName, value);
//}
//
//template<typename TValue>
//void AddValues(rapidjson::Value& json_parent, const std::string& strName, TValue& value, auto... args, auto& allocator)
//{
//    AddValue(json_parent, strName, value);
//    AddValues(json_parent, args..., allocator);
//}
//
//void AddValues(rapidjson::Value& json_parent, auto... args, auto& allocator)
//{
//    AddValues(json_parent, args..., allocator);
//}
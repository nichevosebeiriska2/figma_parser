#pragma once

#include <filesystem>
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

bool SaveJsonToFile(const rapidjson::Value& value,  std::filesystem::path pathToFile);
bool SaveJsonToFile(const rapidjson::Document& doc, std::filesystem::path pathToFile);
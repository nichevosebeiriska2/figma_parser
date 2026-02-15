#include <fstream>

#include "json_utilities.h"
#include "rapidjson/prettywriter.h"

bool SaveJsonToFile(const rapidjson::Value& value, std::filesystem::path pathToFile)
{
	std::ofstream file(pathToFile);

	if (!file.is_open())
		return false;

	rapidjson::StringBuffer buffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
	value.Accept(writer);

	file << buffer.GetString();

	return !file.fail();
}


bool SaveJsonToFile(const rapidjson::Document& doc, std::filesystem::path pathToFile)
{
	std::ofstream file(pathToFile);

	if (!file.is_open())
		return false;

	rapidjson::StringBuffer buffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);

	file << buffer.GetString();

	return !file.fail();
}
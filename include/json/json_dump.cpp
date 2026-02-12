#include "json_dump.h"

using namespace rapidjson;

rapidjson::Value JsonVisitorNull::Serialize(const tKiwiValue& kiwi_value)
{
	return Value();
}

rapidjson::Value JsonVisitorBool::Serialize(const tKiwiValue& kiwi_value)
{
	return Value();
}

rapidjson::Value JsonVisitorInt::Serialize(const tKiwiValue& kiwi_value)
{
	return Value();
}

rapidjson::Value JsonVisitorFloat::Serialize(const tKiwiValue& kiwi_value)
{
	return Value();
}

rapidjson::Value JsonVisitorString::Serialize(const tKiwiValue& kiwi_value)
{
	return Value();
}

rapidjson::Value JsonVisitorArray::Serialize(const tKiwiValue& kiwi_value)
{
	return Value();
}

rapidjson::Value JsonVisitorObject::Serialize(const tKiwiValue& kiwi_value)
{
	return Value();
}


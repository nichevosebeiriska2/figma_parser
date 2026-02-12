#pragma once

#include <cmath>

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"

#include "kiwi_types.h"
#include "kiwi_scheme_decoder.h"
#include "json_utilities.h"

class JsonSchemePrinter
{
protected:
	auto SchemeTypeToJson(auto& scheme, const KiwiTypeScheme& type, auto& allocator) const
	{
		using namespace rapidjson;
		Value object(kObjectType);

		const std::string strKind = ComplexDataTypeToString(static_cast<EEntityKind>(type.m_iKind));

		Value arr_fields(kArrayType);

		std::string strDataType;
		strDataType.reserve(128);

		for (auto& [field_id, field] : type.m_mapFields)
		{
			Value object_field(kObjectType);

			
			if (field.m_iType < 0)
				strDataType = PrimitiveDataTypeToString(static_cast<EPrimitiveDataType>(~field.m_iType));
			else
			{
				const auto& type = scheme.GetTypeById(field.m_iType);
				strDataType = ComplexDataTypeToString(static_cast<EEntityKind>(type.m_iKind)) + "(" + field.m_strName + ")";
			}

			AddValues(object_field, allocator
								  , std::string{ "name" },	field.m_strName
								  , std::string{ "array" }, field.m_bArray
								  , std::string{ "type" },	strDataType
								  , std::string{ "value" }, field.m_Value);

			arr_fields.PushBack(object_field, allocator);
		}

		AddValues(object, allocator, std::string{ "name" }, type.m_strName, "kind", strKind, std::string{ "fields" }, std::move(arr_fields));

		return std::move(object);
	}
public:
	auto Print(KiwiScheme& scheme) const
	{
		using namespace rapidjson;
		Document doc(kArrayType);
		auto& allocator = doc.GetAllocator();

		for (const KiwiTypeScheme& type : scheme.GetTypes())
			doc.PushBack(SchemeTypeToJson(scheme, type, allocator), allocator);

		return doc;
	}
};


template<typename TJsonAllocator>
class JsonKiwiPrinter
{
	using tAllocator = rapidjson::GenericDocument<rapidjson::UTF8<>, TJsonAllocator>;
	TJsonAllocator& m_allocator;
	
	tAllocator m_doc;

public:

	JsonKiwiPrinter(TJsonAllocator& allocator)
		: m_allocator{allocator}
		, m_doc(&m_allocator)
	{
	}

	rapidjson::Document Print(tKiwiValue& root_message);

	auto operator()(auto&& arg) const
	{
		using tArgType = std::remove_cvref_t <decltype(arg)> ;
		if constexpr (std::is_same_v<tArgType, std::monostate>)
		{
			return TJsonValue();
		}
		else if constexpr (std::is_same_v<tArgType, bool>)
		{
			return TJsonValue(arg);
		}
		else if constexpr (std::is_same_v<tArgType, BYTE>)
		{
			return TJsonValue((int)arg);
		}
		else if constexpr (std::is_same_v<tArgType, INT>)
		{
			return TJsonValue(arg);
		}
		else if constexpr (std::is_same_v<tArgType, UINT>)
		{
			return TJsonValue(arg);
		}
		else if constexpr (std::is_same_v<tArgType, float>)
		{
			if(std::isnan(arg))
				return TJsonValue("nan");
			else if(std::isinf(arg))
				return TJsonValue("inf");

			return TJsonValue(arg);
		}
		else if constexpr (std::is_same_v<tArgType, INT64>)
		{
			return TJsonValue(arg);
		}
		else if constexpr (std::is_same_v<tArgType, UINT64>)
		{
			return TJsonValue(arg);
		}
		else if constexpr (std::is_same_v<tArgType, std::string>)
		{
			return TJsonValue(arg.c_str(), arg.length(), m_allocator);
		}
		else if constexpr (std::is_same_v<tArgType, uPtr<sEnum>>)
		{
			return TJsonValue(arg->m_strValue.c_str(), arg->m_strValue.length(), m_allocator);
		}
		else if constexpr (std::is_same_v<tArgType, uPtr<sStruct>>)
		{
			TJsonValue structure(rapidjson::kObjectType);
			JsonKiwiPrinter printer(m_allocator);
			for(auto &[name, kiwi_value] : arg->m_mapValues)
				structure.AddMember(TJsonValue(name.c_str(), name.length(), m_allocator), std::visit(printer, kiwi_value), m_allocator);

			return structure;
		}
		else if constexpr (std::is_same_v<tArgType, uPtr<sMessage>>)
		{
			TJsonValue message(rapidjson::kObjectType);
			JsonKiwiPrinter printer(m_allocator);
			for (auto& [name, kiwi_value] : arg->m_mapValues)
				message.AddMember(TJsonValue(name.c_str(), name.length(), m_allocator), std::visit(printer, kiwi_value), m_allocator);
			
			return message;
		}
		else if constexpr (std::is_same_v<tArgType, uPtr<sArray>>)
		{
			TJsonValue array(rapidjson::kArrayType);
			JsonKiwiPrinter printer(m_allocator);
			for(auto &kiwi_value : arg->m_vecValues)
				array.PushBack(std::visit(printer, kiwi_value), m_allocator);

			return array;
		}
		else
			static_assert(false, "JsonKiwiPrinter::Error - no case for one of tKiwiValue internal types");
	}

	//auto operator()(tKiwiValue& kiwi_value) const
	//{
	//	return 0;
	//}

	//auto operator()(bool b) const
	//{
	//	//return b;
	//	return 0;
	//}

	//auto operator()(BYTE b) const
	//{
	//	//return b;
	//	return 0;
	//}

	//auto operator()(INT i) const
	//{
	//	//return i;
	//	return 0;
	//}

	//auto operator()(UINT i) const
	//{
	//	//return i;
	//	return 0;
	//}

	//auto operator()(float i) const
	//{
	//	//return i;
	//	return 0;
	//}

	//auto operator()(std::string i) const
	//{
	//	//return i;
	//	return 0;
	//}

	//auto operator()(INT64 i) const
	//{
	//	//return i;
	//	return 0;
	//}

	//auto operator()(UINT64 i) const
	//{
	//	//return i;
	//	return 0;
	//}

	//auto operator()(uPtr<sEnum> i) const
	//{
	//	//return i;
	//	return 0;
	//}

	//auto operator()(uPtr<sMessage> i) const
	//{
	//	//return i;
	//	return 0;
	//}

	//auto operator()(uPtr<sStruct> i) const
	//{
	//	//return i;
	//	return 0;
	//}
};

class JsonVisitor
{
	virtual rapidjson::Value Serialize(const tKiwiValue& kiwi_value) = 0;
};

class JsonVisitorNull
{
	rapidjson::Value Serialize(const tKiwiValue& kiwi_value);
};

class JsonVisitorBool
{
	rapidjson::Value Serialize(const tKiwiValue& kiwi_value);
};

class JsonVisitorInt
{
	rapidjson::Value Serialize(const tKiwiValue& kiwi_value);
};

class JsonVisitorFloat
{
	rapidjson::Value Serialize(const tKiwiValue& kiwi_value);
};

class JsonVisitorString
{
	rapidjson::Value Serialize(const tKiwiValue& kiwi_value);
};

class JsonVisitorArray
{
	rapidjson::Value Serialize(const tKiwiValue& kiwi_value);
}; 

class JsonVisitorObject
{
	rapidjson::Value Serialize(const tKiwiValue& kiwi_value);
};




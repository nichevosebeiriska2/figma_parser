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
	auto SchemeTypeToJson(auto& scheme, const KiwiSchemeType& type, auto& allocator) const
	{
		using namespace rapidjson;
		Value object(kObjectType);
		Value arrayFields(kArrayType);

		std::string_view strDataType;
		std::string_view strKind = ComplexDataTypeToString(static_cast<EEntityKind>(type.m_eKind));


		for (auto& [field_id, field] : type.m_mapFields)
		{
			Value objectField(kObjectType);

			
			if (field.m_iType < 0)
				strDataType = PrimitiveDataTypeToString(static_cast<EPrimitiveDataType>(~field.m_iType));
			else
			{
				const auto& type = scheme.GetTypeById(field.m_iType);
				strDataType = ComplexDataTypeToString(type.m_eKind);
			}

			// it effective to use string referenses due to guaranteed scheme lifetime
			objectField.AddMember("name", rapidjson::StringRef(field.m_strName.data(), field.m_strName.length()), allocator);
			objectField.AddMember("type", rapidjson::StringRef(strDataType.data(), strDataType.length()), allocator);
			objectField.AddMember("array", field.m_bArray, allocator);
			objectField.AddMember("value", field.m_Value, allocator);

			arrayFields.PushBack(std::move(objectField), allocator);
		}

		object.AddMember("name",	rapidjson::Value(type.m_strName.data(), type.m_strName.length()), allocator);
		object.AddMember("kind",	rapidjson::Value(strKind.data(), strKind.length()), allocator);
		object.AddMember("fields",	std::move(arrayFields),	allocator);

		return std::move(object);
	}
public:
	auto Print(KiwiScheme& scheme) const
	{
		using namespace rapidjson;
		Document doc(kArrayType);
		auto& allocator = doc.GetAllocator();

		for (const KiwiSchemeType& type : scheme.GetTypes())
			doc.PushBack(SchemeTypeToJson(scheme, type, allocator), allocator);

		return doc;
	}
};


template<typename TJsonAllocator = rapidjson::Document::AllocatorType>
class JsonKiwiPrinter
{
	using TAllocator = rapidjson::Document::AllocatorType;
	using TJsonValue = rapidjson::Value;

	TAllocator* m_allocator;

public:

	JsonKiwiPrinter()
		: m_allocator{ new TAllocator() }
	{
	}

	JsonKiwiPrinter(TAllocator* allocator)
		: m_allocator{allocator}
	{
	}

	TJsonValue operator()(auto&& arg) const
	{
		using tArgType = std::remove_cvref_t <decltype(arg)> ;

		if constexpr (std::is_same_v<tArgType, std::monostate>) // dont know exactly 
			return TJsonValue(rapidjson::kNullType);

		else if constexpr (std::is_integral_v<tArgType>) // BYTE/INT/UINT/INT64/UINT64
			return TJsonValue(arg);

		else if constexpr (std::is_same_v<tArgType, float>)
		{
			// sometimes decoder return nan/inf as a result of float parsing. its not clear how to parse float properly. 
			// ocurrs with some pattern like for specific field only. is it even an error? 
			// json writer would go down on nan/inf values so handle this cases as a string value
			if (std::isnan(arg))
				return TJsonValue("nan");
			else if (std::isinf(arg))
				return TJsonValue("inf");

			return TJsonValue(arg);
		}

		else if constexpr (std::is_same_v<tArgType, std::string> || std::is_same_v<tArgType, std::string_view>)
			return TJsonValue(arg.data(), arg.length(), *m_allocator);

		//else if constexpr (std::is_same_v<tArgType, uPtr<SEnum>>) // SEnum replaced with string_view to kiwi scheme enum value
		//	return TJsonValue(rapidjson::StringRef(arg->m_strValue.data()));

		else if constexpr (std::is_same_v<tArgType, uPtr<SStruct>> || std::is_same_v<tArgType, uPtr<SMessage>>)
		{
			TJsonValue object(rapidjson::kObjectType);
			JsonKiwiPrinter printer(m_allocator);
			for(auto &[name, kiwi_value] : arg->m_mapValues)
				object.AddMember(TJsonValue(rapidjson::StringRef(name.data())), std::visit(printer, kiwi_value), *m_allocator);

			return object;
		}
		else if constexpr (std::is_same_v<tArgType, uPtr<SArray>>)
		{
			TJsonValue array(rapidjson::kArrayType);
			JsonKiwiPrinter printer(m_allocator);
			for(auto &kiwi_value : arg->m_vecValues)
				array.PushBack(std::visit(printer, kiwi_value), *m_allocator);

			return array;
		}
		else
			static_assert(false, "JsonKiwiPrinter::Error - no case for one of tKiwiValue internal types");
	}
};
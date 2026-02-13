
#include <type_traits>

#include "kiwi_decoder.h"

tKiwiValue KiwiDecoder::DecodePrimitiveType(KiwiReader &reader, UINT iType)
{
	switch(iType)
	{
		case (EPrimitiveDataType::EPrimitiveDataTypeBool):
			return reader.GetBool();
		case (EPrimitiveDataType::EPrimitiveDataTypeByte):
			return reader.GetByte();
		case (EPrimitiveDataType::EPrimitiveDataTypeInt):
			return reader.GetInt();
		case (EPrimitiveDataType::EPrimitiveDataTypeUint):
			return reader.GetUint();
		case (EPrimitiveDataType::EPrimitiveDataTypeFloat):
			return reader.GetFloat();
		case (EPrimitiveDataType::EPrimitiveDataTypeString):
			return reader.GetString();
		case (EPrimitiveDataType::EPrimitiveDataTypeInt64):
			return reader.GetInt64();
		case(EPrimitiveDataType::EPrimitiveDataTypeUint64) :
			return reader.GetUint64();
	}
	return std::monostate{};
}


tKiwiValue KiwiDecoder::DecodeTypeInner(KiwiReader &reader, int iFieldType, bool bArray)
{
	if(bArray)
	{
		const UINT iNumOfElements = reader.GetUint();

		auto pArray = std::make_unique<sArray>();
		pArray->m_vecValues.reserve(iNumOfElements);

		for(int i = 0; i < iNumOfElements; i++)
			pArray->m_vecValues.emplace_back(DecodeTypeInner(reader, iFieldType, false));

		return std::move(pArray);
	}

	if(iFieldType < 0)
		return DecodePrimitiveType(reader, ~iFieldType);
	
	else
	{
		const KiwiTypeScheme& field = m_scheme.GetTypeById(iFieldType);

		switch(field.m_iKind)
		{
			case (0):
				return DecodeEnum(reader, field);

			case (1):
				return DecodeStruct(reader, field);

			case (2):
				return DecodeMessage(reader, field);

			default:
				break;
		}
	}


	return -1;
}


tKiwiValue KiwiDecoder::DecodeEnum(KiwiReader &reader, const KiwiTypeScheme& type)
{
	return std::string_view{type.m_mapFields.at(reader.GetUint()).m_strName};
}


tKiwiValue KiwiDecoder::DecodeStruct(KiwiReader &reader, const KiwiTypeScheme& type)
{
	uPtr<sStruct> sPtrStruct = std::make_unique<sStruct>();
	for(const auto &[field_id, field] : type.m_mapFields)
		sPtrStruct->m_mapValues.emplace(std::make_pair(std::string_view{field.m_strName}, DecodeTypeInner(m_reader, field.m_iType, field.m_bArray)));

	return std::move(sPtrStruct);
}

tKiwiValue KiwiDecoder::DecodeMessage(KiwiReader &reader, const KiwiTypeScheme &type_root)
{
	auto field_id = reader.GetUint();

	uPtr<sMessage> sPtrMessage = std::make_unique<sMessage>();
	while(field_id != 0)
	{
		const sKiwiField& field = type_root.m_mapFields.at(field_id);

		sPtrMessage->m_mapValues.emplace(std::make_pair(std::string_view{field.m_strName}, DecodeTypeInner(m_reader, field.m_iType, field.m_bArray)));
		field_id = reader.GetUint();
	}

	return std::move(sPtrMessage);
}

KiwiDecoder::KiwiDecoder(KiwiScheme&& scheme, TVectorData&& vecDataSecondChunk)
	: m_scheme{std::move(scheme)}
	, m_reader{std::move(vecDataSecondChunk)}
{}

tKiwiValue KiwiDecoder::Decode()
{
	return DecodeMessage(m_reader, m_scheme.FindRootType("Message"));
}

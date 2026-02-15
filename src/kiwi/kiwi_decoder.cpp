
#include <type_traits>
#include <stdexcept>

#include "kiwi_decoder.h"

TKiwiValue KiwiDecoder::DecodePrimitiveType(KiwiReader &reader, UINT iType)
{
	switch(iType)
	{
		case (EPrimitiveDataType::EPrimitiveDataTypeBool):
			return reader.GetBool();
		case (EPrimitiveDataType::EPrimitiveDataTypeByte):
			return reader.GetByte();
		case (EPrimitiveDataType::EPrimitiveDataTypeInt):
			return reader.GetInt64();
		case (EPrimitiveDataType::EPrimitiveDataTypeUint):
			return reader.GetUint64();
		case (EPrimitiveDataType::EPrimitiveDataTypeFloat):
			return reader.GetFloat();
		case (EPrimitiveDataType::EPrimitiveDataTypeString):
			return reader.GetString();
		case (EPrimitiveDataType::EPrimitiveDataTypeInt64):
			return reader.GetInt64();
		case(EPrimitiveDataType::EPrimitiveDataTypeUint64) :
			return reader.GetUint64();
	}

	throw std::out_of_range("KiwiDecoder::DecodePrimitiveType() : invalid primitive type id. fatal error");
}


TKiwiValue KiwiDecoder::DecodeTypeInner(KiwiReader &reader, int iFieldType, bool bArray)
{
	if(bArray)
	{
		const UINT iNumOfElements = reader.GetUint();

		auto pArray = std::make_unique<SArray>();
		pArray->m_vecValues.reserve(iNumOfElements);

		for(int i = 0; i < iNumOfElements; i++)
			pArray->m_vecValues.emplace_back(DecodeTypeInner(reader, iFieldType, false));

		return std::move(pArray);
	}

	else if(iFieldType < 0)
		return DecodePrimitiveType(reader, ~iFieldType);
	
	else
	{
		const KiwiSchemeType& field = m_scheme.GetTypeById(iFieldType);

		switch(field.m_eKind)
		{
			case (EEntityKindEnum):
				return DecodeEnum(reader, field);
			case (EEntityKindStruct):
				return DecodeStruct(reader, field);
			case (EEntityKindMessage):
				return DecodeMessage(reader, field);
			default:
				throw std::out_of_range("KiwiDecoder::DecodeTypeInner() : invalid entity kind id. fatal error");
		}
	}

	throw std::out_of_range("KiwiDecoder::DecodeTypeInner() : value is not array/primitive type/enum/struc/message. fatal error");
}


TKiwiValue KiwiDecoder::DecodeEnum(KiwiReader &reader, const KiwiSchemeType& type)
{
	return std::string_view{type.m_mapFields.at(reader.GetUint()).m_strName};
}


TKiwiValue KiwiDecoder::DecodeStruct(KiwiReader &reader, const KiwiSchemeType& type)
{
	uPtr<SStruct> sPtrStruct = std::make_unique<SStruct>();
	for (const auto& [field_id, field] : type.m_mapFields)
		sPtrStruct->m_mapValues[field.m_strName] = DecodeTypeInner(m_reader, field.m_iType, field.m_bArray);

	return sPtrStruct;
}

TKiwiValue KiwiDecoder::DecodeMessage(KiwiReader &reader, const KiwiSchemeType &type_root)
{
	UINT iFieldId;

	uPtr<SMessage> sPtrMessage = std::make_unique<SMessage>();
	while(iFieldId = reader.GetUint())
	{
		const sKiwiField& field = type_root.m_mapFields.at(iFieldId);
		sPtrMessage->m_mapValues[field.m_strName] = DecodeTypeInner(m_reader, field.m_iType, field.m_bArray);
	}

	return sPtrMessage;
}

KiwiDecoder::KiwiDecoder(KiwiScheme&& scheme, TVectorData&& vecDataSecondChunk)
	: m_scheme{std::move(scheme)}
	, m_reader{std::move(vecDataSecondChunk)}
{
	Decode();
}

void KiwiDecoder::Decode()
{ 
	m_rootMessage = DecodeMessage(m_reader, m_scheme.FindRootType("Message"));
}


TKiwiValue& KiwiDecoder::GetRootMessage()
{
	return m_rootMessage;
}


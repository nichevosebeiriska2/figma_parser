
#include <type_traits>

#include "kiwi_scheme_decoder.h"
#include "allocation.h"

KiwiScheme::KiwiScheme(TVectorData &&vecDataFirstChunk)
	: m_vecDataScheme{std::move(vecDataFirstChunk)}
{
}


bool KiwiScheme::Decode()
{
	KiwiReader kwReaderScheme(std::move(m_vecDataScheme));
	DecodeScheme(kwReaderScheme);

	return true;
}

void KiwiScheme::DecodeScheme(KiwiReader &reader)
{
	const UINT iTypes = reader.GetUint();
	for(int i = 0; i < iTypes; i++)
	{
		KiwiTypeScheme type
		{
			.m_strName = reader.GetString(),
			.m_iKind = reader.GetByte()
		};

		auto &mapFields = type.m_mapFields;

		const UINT iNumOfFields = reader.GetUint();
		for(int i = 0; i < iNumOfFields; i++)
		{
			sKiwiField sField
			{
				.m_strName	= reader.GetString(),
				.m_iType	= reader.GetInt(),
				.m_bArray	= reader.GetBool(),
				.m_Value	= reader.GetUint(),
			};

			mapFields[sField.m_Value] = sField;
			//vecFields.emplace_back(sField);
		}


		m_vecTypes.emplace_back(type);
	}
}


tVecTypes KiwiScheme::GetTypes()
{
	return m_vecTypes;
}

const KiwiTypeScheme& KiwiScheme::GetTypeById(const UINT id)
{
	return m_vecTypes[id];
}


const KiwiTypeScheme &KiwiScheme::FindRootType(const std::string &strRootSchemeName)
{
	for(const auto &type : m_vecTypes)
	{
		if(type.m_strName == strRootSchemeName)
			return type;
	}
}

tKiwiValue KiwiDecoder::DecodeMessage(KiwiReader &reader, const KiwiTypeScheme &type_root)
{
	auto field_id = reader.GetUint();

	//std::make_unique<int>(
	uPtr<sMessage> sPtrMessage = std::make_unique<sMessage>();
	while(field_id != 0)
	{
		auto field = type_root.m_mapFields.at(field_id);
		int field_type = field.m_iType;
		bool bArray = field.m_bArray;

		sPtrMessage->m_mapValues[field.m_strName] = DecodeType(reader, field_type, bArray);
		//tKiwiValue val = DecodeType(reader, field_type, bArray);

		//auto& f_Val = m_rootMessage.m_mapValues[field.m_strName];
		//f_Val = val;
		field_id = reader.GetUint();
	}

	return sPtrMessage;
}


tKiwiValue KiwiDecoder::DecodePrimitiveType(KiwiReader &reader, UINT iType)
{
	switch(iType)
	{
		case (EPrimitiveDataType::tBool):
			return reader.GetBool();
		case (EPrimitiveDataType::tByte):
			return reader.GetByte();
		case (EPrimitiveDataType::tInt):
			return reader.GetInt();
		case (EPrimitiveDataType::tUint):
			return reader.GetUint();
		case (EPrimitiveDataType::tFloat):
			return reader.GetFloat();
		case (EPrimitiveDataType::tString):
			return reader.GetString();
		case (EPrimitiveDataType::tInt64):
			return reader.GetInt64();
		case(EPrimitiveDataType::tUint64) :
			return reader.GetUint64();
		default:
			break;
	}
	return std::monostate{};
}

tKiwiValue KiwiDecoder::DecodeType(KiwiReader &reader, int iFieldType, bool bArray)
{
	return DecodeTypeInner(reader, iFieldType, bArray);

	// convert by converter
	//{  }

	//return r;
}


tKiwiValue KiwiDecoder::DecodeTypeInner(KiwiReader &reader, int iFieldType, bool bArray)
{
	if(bArray)
	{
		const UINT iNumOfElements = reader.GetUint();

		auto pArray = std::make_unique<sArray>();
		for(int i = 0; i < iNumOfElements; i++)
		{
			auto visitor = [](auto&& arg) {return tKiwiValue{ std::move(arg) };};
			pArray->m_vecValues.emplace_back(std::visit(visitor, DecodeType(reader, iFieldType, false)));
		}

		return pArray;
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
	return type.m_mapFields.at(reader.GetUint()).m_strName;
}


tKiwiValue KiwiDecoder::DecodeStruct(KiwiReader &reader, const KiwiTypeScheme& type)
{
	uPtr<sStruct> sPtrStruct = std::make_unique<sStruct>();
	for (const auto& [field_id, field] : type.m_mapFields)
		sPtrStruct->m_mapValues[field.m_strName] = DecodeType(m_reader, field.m_iType, field.m_bArray);
	
	return sPtrStruct;
}

KiwiDecoder::KiwiDecoder(KiwiScheme&& scheme, TVectorData&& vecDataSecondChunk)
	: m_scheme{std::move(scheme)}
	, m_reader{std::move(vecDataSecondChunk)}
{}

tKiwiValue KiwiDecoder::Decode()
{
	return DecodeMessage(m_reader, m_scheme.FindRootType("Message"));
}

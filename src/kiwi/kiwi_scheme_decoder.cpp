
#include <type_traits>

#include "kiwi_scheme_decoder.h"

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
	return *std::ranges::find(m_vecTypes, strRootSchemeName, &KiwiTypeScheme::m_strName);
}
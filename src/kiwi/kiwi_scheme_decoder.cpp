
#include <type_traits>
#include <stdexcept>
#include <format>

#include "kiwi_scheme_decoder.h"

KiwiScheme::KiwiScheme(TVectorData &&vecDataFirstChunk)
	: m_reader{std::move(vecDataFirstChunk)}
{
	DecodeScheme(m_reader);
}


void KiwiScheme::DecodeScheme(KiwiReader &reader)
{
	const UINT iTypes = reader.GetUint();
	m_vecTypes.reserve(iTypes);
	for(int i = 0; i < iTypes; i++)
	{
		KiwiSchemeType type
		{
			.m_strName = reader.GetString(),
			.m_eKind = static_cast<EEntityKind>(reader.GetByte())
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


const TVecSchemeTypes& KiwiScheme::GetTypes()
{
	return m_vecTypes;
}


const KiwiSchemeType& KiwiScheme::GetTypeById(const UINT id)
{
	if (id >= m_vecTypes.size())
		throw std::out_of_range(std::format("KiwiScheme::GetTypeById() - no type with id {}. fatal error", id));

	return m_vecTypes.at(id);
}


const KiwiSchemeType &KiwiScheme::FindRootType(const std::string &strRootSchemeName)
{
	auto it = std::ranges::find(m_vecTypes, strRootSchemeName, &KiwiSchemeType::m_strName);

	if (it == m_vecTypes.end())
		throw std::out_of_range(std::format("KiwiScheme::FindRootType() - no type with name {}. fatal error", strRootSchemeName));

	return *it;
}
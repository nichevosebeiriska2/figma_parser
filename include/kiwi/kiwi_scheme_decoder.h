#pragma once

#include <map>
#include <variant>
#include "types.h"
#include "kiwi_reader.h"

class KiwiScheme
{
protected:
	TVecSchemeTypes  m_vecTypes;
	KiwiReader m_reader;

protected:
	void DecodeScheme(KiwiReader &reader);

public:
	KiwiScheme(TVectorData&& vecDataFirstChunk);

	const TVecSchemeTypes& GetTypes();
	const KiwiSchemeType& GetTypeById(const UINT id);
	const KiwiSchemeType& FindRootType(const std::string& strRootSchemeName);
};
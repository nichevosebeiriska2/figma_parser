#pragma once

#include <map>
#include <variant>
#include "types.h"
#include "kiwi_reader.h"

class KiwiScheme
{
protected:
	tVecTypes	m_vecTypes;
	TVectorData m_vecDataScheme; // first chunk with kiwi scheme

protected:
	void				DecodeScheme(KiwiReader &reader);

public:
	KiwiScheme(TVectorData&& vecDataFirstChunk);
	bool Decode();

	tVecTypes GetTypes();
	const KiwiTypeScheme& GetTypeById(const UINT id);
	const KiwiTypeScheme& FindRootType(const std::string& strRootSchemeName);

	template<typename TPrinter>
	auto AcceptPrinter(const TPrinter& printer)
	{
		return printer.Print(*this);
	}
};
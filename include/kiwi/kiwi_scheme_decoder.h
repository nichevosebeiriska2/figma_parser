#pragma once

#include <map>
#include <variant>
#include "types.h"
#include "kiwi_reader.h"

class KiwiScheme
{
public:
	//enum EPrimitiveDataType { tBool, tByte, tInt, tUint, tFloat, tString, tInt64, tUint64 };
	//enum EEntityKind { ENUM, STRUCT, MESSAGE };

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

class KiwiDecoder
{
protected:
	KiwiScheme m_scheme;
	KiwiReader m_reader;

	sMessage m_rootMessage;

protected:
	tKiwiValue	DecodePrimitiveType(KiwiReader& reader, UINT iType);
	tKiwiValue	DecodeType(KiwiReader& reader, int iFieldType, bool bArray);
	tKiwiValue	DecodeEnum(KiwiReader& reader, const KiwiTypeScheme& type);
	tKiwiValue	DecodeStruct(KiwiReader& reader, const  KiwiTypeScheme& type);
	tKiwiValue	DecodeTypeInner(KiwiReader& reader, int iFieldType, bool bArray);
	tKiwiValue	DecodeMessage(KiwiReader& reader, const KiwiTypeScheme& type_root);

public:
	KiwiDecoder(KiwiScheme&& scheme, TVectorData&& vecDataSecondChunk);
	tKiwiValue Decode();
};
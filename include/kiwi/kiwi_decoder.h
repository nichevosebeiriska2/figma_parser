#pragma once

#include <map>
#include <variant>
#include "types.h"
#include "kiwi_scheme_decoder.h"

class KiwiDecoder
{
protected:
	KiwiScheme m_scheme;
	KiwiReader m_reader;
	TKiwiValue m_rootMessage;
	bool m_bHasError;

protected:
	TKiwiValue	DecodePrimitiveType(KiwiReader& reader, UINT iType);
	TKiwiValue	DecodeEnum(KiwiReader& reader, const KiwiSchemeType& type);
	TKiwiValue	DecodeStruct(KiwiReader& reader, const  KiwiSchemeType& type);
	TKiwiValue	DecodeTypeInner(KiwiReader& reader, int iFieldType, bool bArray);
	TKiwiValue	DecodeMessage(KiwiReader& reader, const KiwiSchemeType& type_root);
	void Decode();

public:
	KiwiDecoder(KiwiScheme&& scheme, TVectorData&& vecDataSecondChunk);
	
	TKiwiValue& GetRootMessage();
};
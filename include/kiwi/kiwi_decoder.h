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

	sMessage m_rootMessage;

protected:
	tKiwiValue	DecodePrimitiveType(KiwiReader& reader, UINT iType);
	tKiwiValue	DecodeEnum(KiwiReader& reader, const KiwiTypeScheme& type);
	tKiwiValue	DecodeStruct(KiwiReader& reader, const  KiwiTypeScheme& type);
	tKiwiValue	DecodeTypeInner(KiwiReader& reader, int iFieldType, bool bArray);
	tKiwiValue	DecodeMessage(KiwiReader& reader, const KiwiTypeScheme& type_root);

public:
	KiwiDecoder(KiwiScheme&& scheme, TVectorData&& vecDataSecondChunk);
	tKiwiValue Decode();
};
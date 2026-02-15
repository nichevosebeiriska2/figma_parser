#pragma  once

#include "kiwi_types.h"

class KiwiReader
{
protected:
	TVectorData m_data;
	UINT m_iCurrentPosition = 0;

public:
	KiwiReader(TVectorData &&data);
	std::string GetString();

	bool	GetBool();
	BYTE	GetByte();
	INT		GetInt(); //LEB128
	UINT	GetUint();//LEB128
	float	GetFloat();
	INT64	GetInt64();//LEB128
	UINT64	GetUint64();//LEB128

	UINT GetOffset() const;
};

std::string_view PrimitiveDataTypeToString(EPrimitiveDataType eType);
std::string_view ComplexDataTypeToString(EEntityKind eType);
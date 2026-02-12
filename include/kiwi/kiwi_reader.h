
#pragma  once
#include "kiwi_types.h"

class KiwiReader
{
protected:
	tVectorData m_data;
	size_t m_iCurrentPosition = 0;

public:
	KiwiReader(tVectorData &&data);
	std::string GetString();

	bool GetBool();
	BYTE GetByte();
	INT GetInt();
	UINT GetUint();
	float GetFloat();
	INT64 GetInt64();
	UINT64 GetUint64();
};
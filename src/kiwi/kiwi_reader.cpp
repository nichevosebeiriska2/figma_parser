#include "kiwi_reader.h"
#include <cmath>

std::string PrimitiveDataTypeToString(EPrimitiveDataType eType)
{
	switch (eType)
	{
		case EPrimitiveDataTypeBool:
			return "bool";
		case EPrimitiveDataTypeByte:
			return "byte";
		case EPrimitiveDataTypeInt:
			return "int";
		case EPrimitiveDataTypeUint:
			return "uint";
		case EPrimitiveDataTypeFloat:
			return "float";
		case EPrimitiveDataTypeString:
			return "string";
		case EPrimitiveDataTypeInt64:
			return "int64";
		case EPrimitiveDataTypeUint64:
			return "uint64";
	}
	return "";
}

std::string ComplexDataTypeToString(EEntityKind eType)
{
	switch (eType)
	{
		case EEntityKindEnum:
			return "enum";
		case EEntityKindStruct:
			return "structure";
		case EEntityKindMessage:
			return "message";
	}
	return "";
}

KiwiReader::KiwiReader(TVectorData &&data)
	: m_data{data}
{
}

std::string KiwiReader::GetString()
{
	std::string strResult;

	BYTE b = GetByte();
	BYTE b_prev;
	while(!(b == 0x0 /*&& b_prev == 0x0*/))
	{
		strResult += b;
		b_prev = b;
		b = GetByte();
	}

	return strResult;
}

bool KiwiReader::GetBool()
{
	return GetByte() > 0;
}

BYTE KiwiReader::GetByte() // LEB128 op
{
	return m_data[m_iCurrentPosition++];

}
int KiwiReader::GetInt()
{
	UINT i = GetUint();

	return (i & 1)?  ~(i >> 1) : i >> 1;

	if(i & 1)
		return ~(i >> 1);
	else
		return i >> 1;
	//return ~i;
}

UINT KiwiReader::GetUint()
{
	UINT iValue = 0;
	BYTE b = 0;
	for(int i = 0; i < 7; i++)
	{
		b = GetByte();

		iValue |= (b & 127) << i*7; // collect 7 bits of data;

		if(b < 128) // if first bite != 1 -> stop 
			break;
	}

	return iValue;

}


float KiwiReader::GetFloat()
{
	BYTE b = GetByte();

	if(b == 0)
		return 0.0f;

	BYTE b0 = b;
	BYTE b1 = GetByte();
	BYTE b2 = GetByte();
	BYTE b3 = GetByte();

	UINT bits = (b | b1 << 8 | b2 << 16 | b3 << 24);
	bits = (bits << 23) | (bits >> 9);

	float f = *reinterpret_cast<float *>(&bits);

	if(std::isnan(f) || std::isinf(f))
	{
		int a = 1;
	}

	return *reinterpret_cast<float*>(&bits);
}


INT64 KiwiReader::GetInt64()
{
	const UINT i = GetUint64();

	return (i & 1)?  ~(i >> 1) : i >> 1;
}

UINT64 KiwiReader::GetUint64()
{
	UINT iValue = 0;
	BYTE b = 0;
	for(int i = 0; i < 10; i++)
	{
		b = GetByte();

		iValue |= (b & 127) << i*7; // collect 7 bits of data;

		if(b < 128) // if first bite != 1 -> stop 
			break;
	}

	return iValue;
}


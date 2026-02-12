#include "kiwi_reader.h"

std::string PrimitiveDataTypeToString(EPrimitiveDataType eType)
{
	switch (eType)
	{
		case tBool:
			return "bool";
		case tByte:
			return "byte";
		case tInt:
			return "int";
		case tUint:
			return "uint";
		case tFloat:
			return "float";
		case tString:
			return "string";
		case tInt64:
			return "int64";
		case tUint64:
			return "uint64";
	}
	return "";
}

std::string ComplexDataTypeToString(EEntityKind eType)
{
	switch (eType)
	{
		case ENUM:
			return "enum";
		case STRUCT:
			return "structure";
		case MESSAGE:
			return "message";
	}
	return "";
}

KiwiReader::KiwiReader(tVectorData &&data)
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

	UINT bits = (b | GetByte() << 8 | GetByte() << 16 | GetByte() << 24);
	bits = (bits << 23) | (bits >> 9);

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


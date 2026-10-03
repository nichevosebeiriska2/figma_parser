#include "kiwi_reader.h"
#include <cmath>
#include <bitset>

// look 'https://github.com/evanw/kiwi' for more info about kiwi types

std::string_view PrimitiveDataTypeToString(EPrimitiveDataType eType)
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

std::string_view ComplexDataTypeToString(EEntityKind eType)
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
	while(b != 0x0)
	{
		strResult += b;
		b = GetByte();
	}

	return strResult;
}

bool KiwiReader::GetBool()
{
	return GetByte() > 0;
}

BYTE KiwiReader::GetByte()
{
	return m_data.at(m_iCurrentPosition++);

}
int KiwiReader::GetInt()
{
	const UINT i = GetUint();
	return (i & 1)?  ~(i >> 1) : i >> 1;
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
	bits = bits << 23 | bits >> 9;// sometimes it return "nan" 

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


UINT KiwiReader::GetOffset() const
{
	return m_iCurrentPosition;
}
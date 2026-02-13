#pragma once

#include <string>
#include <map>
#include <variant>
#include <memory>
#include <vector>

#include "types.h"


enum EPrimitiveDataType { EPrimitiveDataTypeBool
												, EPrimitiveDataTypeByte
												, EPrimitiveDataTypeInt
												, EPrimitiveDataTypeUint
												, EPrimitiveDataTypeFloat
												, EPrimitiveDataTypeString
												, EPrimitiveDataTypeInt64
												, EPrimitiveDataTypeUint64 };

enum EEntityKind { EEntityKindEnum
								 , EEntityKindStruct
								 , EEntityKindMessage };

std::string PrimitiveDataTypeToString(EPrimitiveDataType eType);
std::string ComplexDataTypeToString(EEntityKind eType);

struct sKiwiField
{
	std::string	m_strName;
	int64_t			m_iType{-1};
	bool				m_bArray{false};
	UINT				m_Value{0};
};

struct KiwiTypeScheme
{
	std::string									m_strName;
	BYTE												m_iKind;
	std::map<UINT, sKiwiField>	m_mapFields;
};

using tVecTypes = std::vector<KiwiTypeScheme>;


struct sEnum;
struct sStruct;
struct sMessage;
struct sArray;


using tKiwiValue = std::variant<  std::monostate
								, bool
								, INT
								, UINT
								, INT64
								, UINT64
								, float
								, std::string
								, std::string_view
								, uPtr<sEnum>
								, uPtr<sStruct>
								, uPtr<sMessage>  
								, uPtr<sArray>>;

// we can use string_view as a map key to decrease std::string memory allocation due to guaranted key lifetime
// (KiwiScheme will be accessible the whole time so type names also will)
using TMap = std::map<std::string_view, tKiwiValue>;
using TVector = std::vector<tKiwiValue>;

struct sEnum
{
	std::string m_strValue;
};

struct sMessage
{
	TMap m_mapValues;

	sMessage()
	{
	}
	sMessage(sMessage &&other) noexcept
		: m_mapValues{std::move(other.m_mapValues)}
	{
	}
};

struct sArray
{
	TVector m_vecValues;
	sArray()
	{
	}
	sArray(sArray &&other) noexcept
		: m_vecValues{std::move(other.m_vecValues)}
	{
	}
};

struct sStruct
{
	TMap m_mapValues;
	sStruct()
	{
	}
	sStruct(sStruct &&other) noexcept
		: m_mapValues{std::move(other.m_mapValues)}
	{}
};

#pragma once

#include <string>
#include <map>
#include <variant>
#include <memory>
#include <vector>


#include "allocation.h"
#include "types.h"


enum EPrimitiveDataType { tBool, tByte, tInt, tUint, tFloat, tString, tInt64, tUint64 };
enum EEntityKind { ENUM, STRUCT, MESSAGE };

std::string PrimitiveDataTypeToString(EPrimitiveDataType eType);
std::string ComplexDataTypeToString(EEntityKind eType);

struct sKiwiField
{
	std::string		m_strName;
	int64_t			m_iType{-1};
	bool			m_bArray = false;
	UINT			m_Value{0};
};

struct KiwiTypeScheme
{
	std::string					m_strName;
	BYTE						m_iKind;
	std::map<UINT, sKiwiField>	m_mapFields;
};

using tVecKiwiFields = std::vector<sKiwiField>;
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
								, uPtr<sEnum>
								, uPtr<sStruct>
								, uPtr<sMessage>  
								, uPtr<sArray>>;


using tMapWithArenaAllocator = std::map < std::string, tKiwiValue, std::less<>, std::pmr::monotonic_buffer_resource>;

struct sEnum
{
	std::string m_strEnumName;
	std::string m_strValue;
};

struct sMessage
{
	std::pmr::map<std::string, tKiwiValue> m_mapValues{&g_arena_allocator};
};

struct sArray
{
	std::pmr::vector<tKiwiValue> m_vecValues{&g_arena_allocator};
};

struct sStruct
{
	std::map<std::string, tKiwiValue> m_mapValues;
};

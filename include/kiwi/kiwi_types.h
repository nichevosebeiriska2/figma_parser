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


struct sKiwiField
{
	std::string	m_strName;
	int64_t		m_iType{-1};
	bool		m_bArray{false};
	UINT		m_Value{0};
};

struct KiwiSchemeType
{
	std::string					m_strName;
	EEntityKind					m_eKind;
	std::map<UINT, sKiwiField>	m_mapFields;
};

using TVecSchemeTypes = std::vector<KiwiSchemeType>;


//struct SEnum;
struct SStruct;
struct SMessage;
struct SArray;

using TKiwiValue =   std::variant<std::monostate	// monostate inside kiwi value definitely should mean some error
								, bool
								, INT
								, UINT
								, INT64
								, UINT64
								, float
								, std::string		// 
								, std::string_view  // there is no string_view like types in kiwi but using string_view highly decreases execution time
								//, uPtr<SEnum>     // sEnum as internal kiwi type replaced by string_view to one of kiwi scheme type enumerator value. it does not really have something usefull 
								, uPtr<SStruct>		// sStruct and sMessage is pretty much the same except messages`s fields are optional
								, uPtr<SMessage>    // 
								, uPtr<SArray>>;	// just a vector of types above. 

// we can use string_view as a map key to decrease std::string memory allocation due to guaranted key lifetime
// (KiwiScheme will be accessible the whole time so type names also will be)
using TMapKiwiFields = std::map<std::string_view, TKiwiValue>;
using TVectorKiwiValues = std::vector<TKiwiValue>;

//struct SEnum
//{
//	std::string_view m_strValue;
//};

struct SMessage
{
	TMapKiwiFields m_mapValues;

	SMessage()
	{
	}
	SMessage(SMessage &&other) noexcept
		: m_mapValues{std::move(other.m_mapValues)}
	{
	}
};

struct SArray
{
	TVectorKiwiValues m_vecValues;
	SArray()
	{
	}
	SArray(SArray &&other) noexcept
		: m_vecValues{std::move(other.m_vecValues)}
	{
	}
};

struct SStruct
{
	TMapKiwiFields m_mapValues;
	SStruct()
	{
	}
	SStruct(SStruct &&other) noexcept
		: m_mapValues{std::move(other.m_mapValues)}
	{
	}
};

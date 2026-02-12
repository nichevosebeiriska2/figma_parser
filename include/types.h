#include <vector>
#include <memory>

using BYTE				= unsigned char;
using tVectorData		= std::vector<BYTE>;
using INT				= int32_t;
using INT64				= int64_t;
using UINT				= uint32_t;
using UINT64			= uint64_t;
template<typename T>
using uPtr = std::unique_ptr<T>;

#include <vector>
#include <string>
#include <optional>
#include <span>


#include <zlib.h>
#include <zstd.h>
#include "types.h"

enum class CompressionFormat { ZLIB, GZIP, RAW_DEFLATE };
using tOptData = std::optional<tVectorData>;

tOptData smart_decompress(std::span<BYTE>& compressed);
tOptData decompress_zstd_chunk(tVectorData &&vecData);
tOptData read_file(const std::string &strPath);

#include <vector>
#include <string>
#include <optional>
#include <span>


#include <zlib.h>
#include <zstd.h>
#include "types.h"

enum class CompressionFormat { ZLIB, GZIP, RAW_DEFLATE };
using tOptData = std::optional<TVectorData>;

tOptData smart_decompress(std::span<BYTE>& compressed);
tOptData decompress_zstd_chunk(TVectorData &&vecData);
tOptData ReadFile(const std::string &strPath);
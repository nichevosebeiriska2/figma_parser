
#include <fstream>
#include <optional>
#include <cstdlib>
#include <print>

#include <zlib.h>
#include <zstd.h>

#include "decompress.h"
#include "types.h"

enum class CompressionFormat { ZLIB, GZIP, RAW_DEFLATE };


CompressionFormat DetectFormat(unsigned char* data)
{
	uint16_t firstTwoBytes = (data[0] << 8) | data[1];

	// if first byte is equal to 78 
	if((firstTwoBytes & 0xFF00) == 0x7800)
		return CompressionFormat::ZLIB;

	// if first 2 bytes eq to 1F 8B
	if ((firstTwoBytes & 0xFFFF) == 0x1F8B)
		return CompressionFormat::GZIP;

	return CompressionFormat::RAW_DEFLATE;
}


bool DecompressChunkZSTD(const std::span<BYTE>& compressedDataView, TVectorData& vecDataOutput)
{
	UINT64 iUncompressedSize = ZSTD_getFrameContentSize(compressedDataView.data(), compressedDataView.size());

	if (iUncompressedSize == ZSTD_CONTENTSIZE_ERROR)
		return false;

	if(iUncompressedSize == ZSTD_CONTENTSIZE_UNKNOWN)
		iUncompressedSize = compressedDataView.size() * 3;

	vecDataOutput.clear();
	vecDataOutput.resize(iUncompressedSize);

	size_t result = ZSTD_decompress(
		vecDataOutput.data(),
		vecDataOutput.size(),
		compressedDataView.data(),
		compressedDataView.size()
	);

	if(ZSTD_isError(result))
		return false;

	vecDataOutput.resize(result);
	return true;
}


bool DecompressChunkZLIB(const std::span<BYTE>& compressedDataView, TVectorData& vecOutputData)
{
	int window_bits = 0;

	switch(DetectFormat(compressedDataView.data()))
	{
		case CompressionFormat::ZLIB:
			window_bits = MAX_WBITS;      // 15
			break;
		case CompressionFormat::GZIP:
			window_bits = 16 + MAX_WBITS; // 31
			break;
		case CompressionFormat::RAW_DEFLATE:
			window_bits = -MAX_WBITS;     // -15
			break;
		default :
			window_bits = MAX_WBITS; // 15 bits by default
	}

	z_stream strm = { 0 };
	if (inflateInit2(&strm, window_bits) != Z_OK)
		return false;

	vecOutputData.clear();
	vecOutputData.resize(compressedDataView.size() * 5);// reserver 5 times of input data

	strm.avail_in = compressedDataView.size();
	strm.next_in = const_cast<Bytef*>(compressedDataView.data());
	strm.avail_out = vecOutputData.size();
	strm.next_out = vecOutputData.data();

	const int ret = inflate(&strm, Z_FINISH);
	inflateEnd(&strm);

	if (ret != Z_STREAM_END)
		return false;

	vecOutputData.shrink_to_fit();

	return true;
}


bool ReadFile(const std::filesystem::path &strPath, TVectorData& vecOutputData)
{
	std::ifstream file(strPath, std::ios::binary);

	if(!file.is_open())
		return false;

	vecOutputData = TVectorData{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>{}};

	return !file.fail();
}

// windows system call to decompress initial .fig file
bool UpzipFile(const std::filesystem::path& pathToFile, const std::filesystem::path& pathOutput)
{
	using namespace std::filesystem;

	if (!exists(pathOutput))
	{
		create_directories(pathOutput);
		std::println("the file path {} did not exist and was created : {}", pathOutput.string(), absolute(pathOutput).string());
	}

	std::string strCmd = "powershell.exe -Command \"Expand-Archive -Path '"
		+ pathToFile.generic_string()
		+ "' -DestinationPath '" + pathOutput.generic_string()
		+ "' -Force\"";

	return std::system(strCmd.c_str()) == 0;
}


CDecompressor::CDecompressor(const CArgumentParser& argParser)
	: m_pathToFigFile{ argParser.GetPathToFigFile()}
{
	if (!UpzipFile(argParser.GetPathToFile(), argParser.GetPathToOutputFile()))
	{
		m_bHasError = true;
		m_strErrorMessage = "CDecompressor::failed to unzip file";
		return;
	}

	if (!ReadFile(m_pathToFigFile, m_vecRawData))
	{
		m_strErrorMessage = std::format("CDecompressor::failed to read .fig file {}", std::filesystem::absolute("failed to read .fig file {}").string());
		m_bHasError = true;
		return;
	}

	if (!m_bHasError && (!DecompressSchemeChunk() || !DecompressMainChunk()))
	{
		m_bHasError = true;
		m_strErrorMessage = "CDecompressor::chunks decompression error";
		return;
	}

	m_vecRawData.clear();
}

TVectorData&& CDecompressor::GetSchemeData()
{
	return std::move(m_vecDataScheme);
}

TVectorData&& CDecompressor::GetMainData()
{
	return std::move(m_vecDataMain);
}

bool CDecompressor::DecompressSchemeChunk()
{
	m_strVigmaHeader = std::string{ m_vecRawData.data(), m_vecRawData.data() + 12 };
	m_iFigmaVersion = *reinterpret_cast<UINT*>(m_vecRawData.data() + 8);

	const UINT iSize = *reinterpret_cast<UINT*>(m_vecRawData.data() + 12);
	m_iSecondChunkOffset = iSize + cUiFigmaHeaderSize;

	if (m_vecRawData.size() < cUiFigmaHeaderSize + iSize)
	{
		m_strErrorMessage = "first chunk size > canvas.fig file size";
		m_bHasError = true;
		return false;
	}

	if (!DecompressChunkZLIB({ m_vecRawData.begin() + cUiFigmaHeaderSize, m_vecRawData.begin() + iSize + cUiFigmaHeaderSize }, m_vecDataScheme))
	{
		m_strErrorMessage = "failed to decompress scheme data chunk";
		m_bHasError = true;
		return false;
	}

	return true;
}

bool CDecompressor::DecompressMainChunk()
{
	const int iNextChunkSize = *reinterpret_cast<int*>(m_vecRawData.data() + m_iSecondChunkOffset);

	if (m_vecRawData.size() < iNextChunkSize + 4 + m_iSecondChunkOffset)
	{
		m_strErrorMessage = "second chunk size reached out of canvas.fig file size";
		m_bHasError = true;
		return false;
	}

	if(! DecompressChunkZSTD(std::span{ m_vecRawData.begin() + m_iSecondChunkOffset + 4
										, m_vecRawData.begin() + m_iSecondChunkOffset + 4 + iNextChunkSize }, m_vecDataMain))
	{
		m_strErrorMessage = "failed to decompress main data chunk";
		m_bHasError = true;
		return false;
	}

	return true;
}

std::string CDecompressor::GetErrorMessage()
{
	return m_strErrorMessage;
}


bool CDecompressor::HasError()
{
	return m_bHasError;
}
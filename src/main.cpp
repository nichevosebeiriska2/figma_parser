#include <string>
#include <fstream>
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <optional>
#include <filesystem>
#include <iostream>

#include <zlib.h>
#include <zstd.h>
#include "kiwi_scheme_decoder.h"
#include "decompress.h"
#include "json_dump.h"
#include "json_utilities.h"
//#include "types.h"

tOptData decompress_first_chunk(tVectorData &vecData, size_t &iIndexMainChunk, UINT *iFigmaVersion = nullptr)
{
	std::string strFigmaHeader{vecData.data(), vecData.data() + 12};

	if(iFigmaVersion)
		*iFigmaVersion = *reinterpret_cast<UINT *>(vecData.data()+8);

	int iSize = *reinterpret_cast<int *>(vecData.data() + 12);
	iIndexMainChunk = 16 + iSize;

	std::span span_to_decompress(vecData.begin() + 12 + 4, vecData.begin() + iSize + 16);

	return smart_decompress(span_to_decompress);
}

tOptData decompress_second_chunk(tVectorData &vecData, size_t iOffset)
{
	bool parsed = false;
	int iNextChunkSize = *reinterpret_cast<int *>(vecData.data() + iOffset);

	std::vector<unsigned char> to_decompress{vecData.begin()+iOffset + 4, vecData.begin()+iOffset+ 4 + iNextChunkSize};
	return decompress_zstd_chunk(std::move(to_decompress));
}

bool SaveSchemeToFile(KiwiScheme& scheme)
{
	JsonSchemePrinter scheme_printer;
	rapidjson::Document DocScheme = scheme.AcceptPrinter(scheme_printer);
	rapidjson::StringBuffer buffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
	DocScheme.Accept(writer);

	std::ofstream file;
	file.open("scheme.json");
	if (file.is_open())
		file << buffer.GetString();

	else
		return false;

	return !file.fail();
}

bool SaveDataToFile(auto &&root_value)
{
	rapidjson::StringBuffer buffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
	root_value.Accept(writer);

	std::ofstream file;
	file.open("data.json");
	if(file.is_open())
		file << buffer.GetString();

	else
		return false;

	return !file.fail();
}


void main(int argc, char** argv)
{
	using namespace rapidjson;

	using namespace std::filesystem;
	
	//argc = 2;
	//argv = char[2]
	//*argv[0] = "";
	//argv[0] = "";

	//if (argc < 2)
	//{
	//	std::cout << "Error : no file specified";
	//	return;
	//}

	path path_to_file{ /*argv[1]*/R"(C:\Users\dmileykin\source\repos\figma_to_json\canvas.fig)"};

	if (!exists(path_to_file))
	{
		std::cout << std::format("Error : file {} does not exists", argv[1]);
		return;
	}


	auto binary_data = read_file(path_to_file.string());

	if (!binary_data)
	{
		std::cout << std::format("Error : failed to read file : {}", argv[1]);
		return;
	}

	size_t iChunkSize = 0;

	auto decompressed_first_chunk = decompress_first_chunk(*binary_data, iChunkSize);

	if (!decompressed_first_chunk)
	{
		std::cout << "Error : failed to decompress first chunk (kiwi scheme chunk) by gzip/zstd";
		return;
	}

	auto decompressed_second_chunk = decompress_second_chunk(*binary_data, iChunkSize);

	if (!decompressed_second_chunk)
	{
		std::cout << "Error : failed to decompress second chunk (kiwi encoded data chunk) by gzip/zstd";
		return;
	}

	KiwiScheme scheme(std::move(*decompressed_first_chunk));
	if (!scheme.Decode())
	{
		int a = 1;
	}
	SaveSchemeToFile(scheme);
	

	KiwiDecoder decoder(std::move(scheme), std::move(*decompressed_second_chunk));
	auto root_message = decoder.Decode();



	JsonArenaAllocator json_allocator;
	JsonKiwiPrinter data_printer(json_allocator);
	TJsonValue result = std::visit(data_printer, root_message);

	if(!SaveDataToFile(result))
	{
		int a = 1;
	}
}
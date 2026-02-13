#include <string>
#include <fstream>
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <optional>
#include <filesystem>
#include <iostream>
#include <print>
#include <expected>

#include <zlib.h>
#include <zstd.h>
#include "kiwi_decoder.h"
#include "decompress.h"
#include "json_dump.h"
#include "json_utilities.h"

std::string_view strErrorFormat = "Error : {}";

tOptData decompress_first_chunk(TVectorData &vecData, size_t &iIndexMainChunk, UINT *iFigmaVersion = nullptr)
{
	std::string strFigmaHeader{vecData.data(), vecData.data() + 12};

	if(iFigmaVersion)
		*iFigmaVersion = *reinterpret_cast<UINT *>(vecData.data()+8);

	int iSize = *reinterpret_cast<int *>(vecData.data() + 12);
	iIndexMainChunk = 16 + iSize;

	std::span span_to_decompress(vecData.begin() + 12 + 4, vecData.begin() + iSize + 16);

	return smart_decompress(span_to_decompress);
}

tOptData decompress_second_chunk(TVectorData &vecData, size_t iOffset)
{
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

void PrintHelpInfo()
{
	std::print(
		"first argument - input .fig file\n"
		"second argument - output directory path(optional). if specified path does not exist print 'Y' to create it"
	);
}

void main(int argc, char** argv)
{
	using namespace std::filesystem;
	
	std::vector<std::string> vecArguments(1);
	for (int i = 0; i < argc-1; i++)
		vecArguments.emplace_back(argv[i+1]);

	//vecArguments[0] = "--help";

	if (vecArguments[0] == "--help")
	{
		PrintHelpInfo();
		return;
	}
	//argc = 2;
	//argv = char[2]
	//*argv[0] = "";
	//argv[0] = "";

	//if (argc < 2)
	//{
	//	std::cout << "Error : no file specified";
	//	return;
	//}

	path path_to_file{R"(C:\Users\dmileykin\source\repos\figma_to_json\canvas.fig)"};
	path path_output{R"(C:\Users\dmileykin\source\repos\figma_parser)"};

	//path path_to_file{ R"(C:\Users\niche\source\repos\figma_to_json\canvas.fig)"};
	//path path_output{ R"(C:\Users\niche\source\repos\figma_to_json\figma_to_json2)" };

	if (!exists(path_to_file))
	{
		std::print("Error : file {} does not exists", path_to_file.string());
		return;
	}

	else if (!exists(path_output))
	{
		std::print("output path \"{}\" does not exists. create it anyway?\nY/n\n", path_output.string());
		std::string strAnswer;
		std::cin >> strAnswer;

		if (strAnswer == "Y" || strAnswer == "y")
			std::filesystem::create_directories(path_output);
		else
		{
			std::print("exit");
			return;
		}
	}

	std::println("extracting .fig data...");

	auto binary_data = ReadFile(path_to_file.string());

	if (!binary_data)
	{
		std::cout << std::format("Error : failed to read file : {}", argv[1]);
		return;
	}

	size_t iChunkSize = 0;


	std::println("decompressing...");

	auto decompressed_first_chunk = decompress_first_chunk(*binary_data, iChunkSize);

	if (!decompressed_first_chunk)
	{
		std::println("failed to decompress first chunk(kiwi scheme chunk) by gzip / zstd");
		return;
	}

	auto decompressed_second_chunk = decompress_second_chunk(*binary_data, iChunkSize);

	if (!decompressed_second_chunk)
	{
		std::println("failed to decompress second chunk (kiwi encoded data chunk) by gzip/zstd");
		return;
	}

	KiwiScheme scheme(std::move(*decompressed_first_chunk));
	if (!scheme.Decode())
	{
		std::println("kiwi scheme decoding error");
		return;
	}

	if (SaveSchemeToFile(scheme))
		std::println("kiwi scheme saved to {}", path_output.string());
	else
	{
		std::println("failed to save kiwi scheme");
		return;
	}

	KiwiDecoder decoder(std::move(scheme), std::move(*decompressed_second_chunk));
	auto root_message = decoder.Decode();

	rapidjson::Document doc;

	JsonKiwiPrinter data_printer(doc.GetAllocator());

	if (SaveDataToFile(std::visit(data_printer, root_message)))
		std::println("kiwi data saved to {}", path_output.string());
	else
		std::println("failed to convert/save kiwi data as json");
}
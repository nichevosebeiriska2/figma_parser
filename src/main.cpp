#include <print>
#include <iostream>

#include "kiwi_decoder.h"
#include "decompress.h"
#include "json_dump.h"
#include "json_utilities.h"
#include "argumentParser.h"


int main(int argc, char** argv)
{
	using namespace std::filesystem;
	
	CArgumentParser argParser(argc, argv);
	if (argParser.HasErrors())
		return -1;

	std::println("extracting .fig data...");

	CDecompressor decompressor(argParser);

	if (decompressor.HasError())
	{
		std::cout<<(decompressor.GetErrorMessage());
		return -1;
	}

	// we can try find type in scheme with invalid type id or read from buffer after it reached its end. 
	// it definitely means some fatal internal error in kiwi deconding. 
	try 
	{
		KiwiScheme scheme(decompressor.GetSchemeData());

		if (SaveJsonToFile(JsonSchemePrinter{}.Print(scheme), argParser.GetPathToOutputSchemeFile()))
			std::println("kiwi scheme saved to {}", std::filesystem::absolute(argParser.GetPathToOutputFile()).string());
		else
		{
			std::println("failed to save kiwi scheme");
			return -1;
		}

		KiwiDecoder decoder(std::move(scheme), decompressor.GetMainData());
		if (SaveJsonToFile(std::visit(JsonKiwiPrinter{}, decoder.GetRootMessage()), argParser.GetPathToOutputDataFile()))
			std::println("kiwi data saved to {}", std::filesystem::absolute(argParser.GetPathToOutputFile()).string());
		else
			std::println("failed to convert/save kiwi data as json");
	}
	catch (std::out_of_range ex)
	{
		std::println("some fatal error occured : {}", ex.what());
	}

	return 0;
}
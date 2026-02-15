#include <print>
#include "argumentParser.h"


CArgumentParser::CArgumentParser(int argc, char** argv)
{
	m_bHasError = ParseArguments(argc, argv);
}


void PrintHelpInfo()
{
	std::println("------------------------------------------------------------------------------------------------------------");
	std::println("first argument - input .fig file saved from figma");
	std::println("second argument - output directory path(optional). if path was not specified it will be \"input path / output\"");
	std::println("------------------------------------------------------------------------------------------------------------");
}


bool CArgumentParser::ParseArguments(int argc, char** argv)
{
	using namespace std::filesystem;

	if (argc == 2 && argv[1] == "--help")
	{
		PrintHelpInfo();
		return true;
	}

	for (int i = 0; i < argc - 1; i++)
		m_vecArguments.emplace_back(argv[i + 1]);

	if (m_vecArguments.empty())
	{
		std::println("no arguments passed - error");
		return true;
	}

	if (m_vecArguments.front() == "--help")
	{
		PrintHelpInfo();
		return true;
	}

	m_pathToFile = m_vecArguments[0];

	if (!exists(m_pathToFile))
	{
		std::println("file{} does not exists", m_pathToFile.string());
		return true;
	}
	else if (m_pathToFile.has_extension() && m_pathToFile.extension().string() != "zip")
	{
		auto pathOld = m_pathToFile;
		rename(pathOld, m_pathToFile.replace_extension("zip"));
	}

	if (m_vecArguments.size() < 2)
		std::println("output path was not specified and replaced by {}", m_pathToFile.parent_path().string());

	m_pathOutput = m_vecArguments.size() > 1 ? m_vecArguments[1] : m_pathToFile.parent_path();

	return false;
}

bool CArgumentParser::HasErrors() const
{
	return m_bHasError;
}


std::filesystem::path CArgumentParser::GetPathToOutputFile() const
{
	return m_pathOutput / "output";
}


std::filesystem::path CArgumentParser::GetPathToOutputSchemeFile() const
{
	return GetPathToOutputFile() / "scheme.json";
}


std::filesystem::path CArgumentParser::GetPathToOutputDataFile() const
{
	return GetPathToOutputFile() / "data.json";
}


std::filesystem::path CArgumentParser::GetPathToFigFile() const
{
	return GetPathToOutputFile() / "canvas.fig";
}


std::filesystem::path CArgumentParser::GetPathToFile() const
{
	return m_pathToFile;
}
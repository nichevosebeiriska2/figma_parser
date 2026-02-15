#pragma once

#include <filesystem>
#include <vector>

class CArgumentParser
{
	std::filesystem::path m_pathToFile;
	std::filesystem::path m_pathOutput;

	std::vector<std::string> m_vecArguments;

	bool m_bHasError{ false };

protected:
	bool ParseArguments(int argc, char** argv);

public:
	CArgumentParser(int argc, char** argv);
	bool HasErrors() const;

	std::filesystem::path GetPathToFigFile() const;
	std::filesystem::path GetPathToFile() const;
	std::filesystem::path GetPathToOutputFile() const;
	std::filesystem::path GetPathToOutputSchemeFile() const;
	std::filesystem::path GetPathToOutputDataFile() const;

};
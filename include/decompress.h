
#include <vector>
#include <string>
#include <optional>
#include <span>
#include <filesystem>

#include "types.h"
#include "argumentParser.h"

class CDecompressor
{
	TVectorData m_vecDataScheme;
	TVectorData m_vecDataMain;
	TVectorData m_vecRawData;

	std::string m_strErrorMessage;
	std::filesystem::path m_pathToFigFile;
	bool m_bHasError{ false };
	std::string m_strVigmaHeader;
	UINT m_iFigmaVersion{ 0 };
	UINT m_iSecondChunkOffset{ 0 };

	constexpr static UINT cUiFigmaHeaderSize{ 16 };

protected:
	bool DecompressSchemeChunk();
	bool DecompressMainChunk();

public:
	CDecompressor(const CArgumentParser& argParser);

	TVectorData&& GetSchemeData();
	TVectorData&& GetMainData();
	std::string GetErrorMessage();
	bool HasError();
};
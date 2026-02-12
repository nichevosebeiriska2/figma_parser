
#include "decompress.h"
#include "types.h"

#include <fstream>
#include <optional>


std::vector<char> decompress_zstd_file(const std::string &input_path)
{
	// 1. Читаем сжатый файл
	std::ifstream in(input_path, std::ios::binary | std::ios::ate);
	if(!in)
	{
		throw std::runtime_error("Не удалось открыть файл: " + input_path);
	}

	std::streamsize size = in.tellg();
	in.seekg(0, std::ios::beg);

	std::vector<char> compressed(size);
	if(!in.read(compressed.data(), size))
	{
		throw std::runtime_error("Ошибка чтения файла");
	}

	// 2. Определяем размер распакованных данных (если известен из заголовка)
	unsigned long long uncompressed_size = ZSTD_getFrameContentSize(
		compressed.data(),
		compressed.size()
	);

	if(uncompressed_size == ZSTD_CONTENTSIZE_ERROR)
	{
		throw std::runtime_error("Невалидный zstd-фрейм");
	}

	// Если размер неизвестен (потоковая передача), используем эвристику
	if(uncompressed_size == ZSTD_CONTENTSIZE_UNKNOWN)
	{
		uncompressed_size = compressed.size() * 3;  // 3x — разумная оценка
	}

	// 3. Выделяем буфер для распакованных данных
	std::vector<char> decompressed(uncompressed_size);

	// 4. Распаковываем
	size_t result = ZSTD_decompress(
		decompressed.data(),
		decompressed.size(),
		compressed.data(),
		compressed.size()
	);

	// 5. Проверяем ошибки
	if(ZSTD_isError(result))
	{
		throw std::runtime_error(
			std::string("Ошибка распаковки: ") + ZSTD_getErrorName(result)
		);
	}

	// 6. Обрезаем буфер до реального размера
	decompressed.resize(result);
	return decompressed;
}

CompressionFormat detect_format(unsigned char* data)
{
	uint16_t cmf_flg = (data[0] << 8) | data[1];

	// Проверка zlib: (CMF << 8) + FLG должно делиться на 31
	if(cmf_flg % 31 == 0 && (data[0] & 0x0F) == 8)
		return CompressionFormat::ZLIB;

	// if first 2 bytes eq to 1F 8B
	if(data[0] == 0x1F && data[1] == 0x8B)
		return CompressionFormat::GZIP;

	return CompressionFormat::RAW_DEFLATE;
}


tOptData decompress_zstd_chunk(tVectorData && compressed)
{
	unsigned long long uncompressed_size = ZSTD_getFrameContentSize(compressed.data(), compressed.size());

	if (uncompressed_size == ZSTD_CONTENTSIZE_ERROR)
		return std::nullopt;

	// Если размер неизвестен (потоковая передача), используем эвристику
	if(uncompressed_size == ZSTD_CONTENTSIZE_UNKNOWN)
	{
		uncompressed_size = compressed.size() * 3;  // 3x — разумная оценка
	}

	// 3. Выделяем буфер для распакованных данных
	std::vector<unsigned char> decompressed(uncompressed_size);

	// 4. Распаковываем
	size_t result = ZSTD_decompress(
		decompressed.data(),
		decompressed.size(),
		compressed.data(),
		compressed.size()
	);

	// 5. Проверяем ошибки
	if(ZSTD_isError(result))
	{
		throw std::runtime_error(
			std::string("Ошибка распаковки: ") + ZSTD_getErrorName(result)
		);
	}

	// 6. Обрезаем буфер до реального размера
	decompressed.resize(result);
	return decompressed;
}


tOptData smart_decompress(std::span<BYTE>& compressed)
{
	z_stream strm = {0};
	int window_bits = MAX_WBITS;  // 15 по умолчанию

	auto format = detect_format(compressed.data());
	switch(format)
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
	}

	// Инициализация с выбранным форматом
	if(inflateInit2(&strm, window_bits) != Z_OK)
		return std::nullopt;

	// Распаковка (как в базовом примере)
	strm.avail_in = compressed.size();
	strm.next_in = const_cast<Bytef *>(compressed.data());

	tVectorData decompressed(compressed.size() * 5); // reserver 5 times of input data. its 
	strm.avail_out = decompressed.size();
	strm.next_out = decompressed.data();

	const int ret = inflate(&strm, Z_FINISH);
	inflateEnd(&strm);

	if(ret != Z_STREAM_END)
		return std::nullopt;

	decompressed.shrink_to_fit();

	return decompressed;
}

std::optional<tVectorData> read_file(const std::string &strPath)
{
	std::ifstream file(strPath, std::ios::binary);

	if(!file.is_open())
		return std::nullopt;

	return tVectorData{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>{}};
}
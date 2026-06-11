#ifndef RLE_H
#define RLE_H

#include <string>
#include <stdexcept>

class RleException : public std::exception {
protected:
	std::string message;
public:
	RleException(const std::string& msg) : message(msg) {}
	const char* what() const noexcept override { return message.c_str(); }
	const std::string& getMessage() const { return message; }
};

class InvalidRleCommand : public RleException {
public:
	InvalidRleCommand(const std::string& msg) : RleException("Invalid RLE command: " + msg) {}
};

class CorruptedMetadata : public RleException {
public:
	CorruptedMetadata(const std::string& msg) : RleException("Corrupted metadata: " + msg) {}
};

class RleCoder {
private:
	static constexpr unsigned char RUN_IDENTICAL_BIT = 0x80;
	static constexpr unsigned char MAX_RUN_IDENTICAL = 129;
	static constexpr unsigned char MAX_RUN_DIFFERENT = 128;

	struct Metadata {
		uint32_t originalSize;
		char originalFilename[256];
	};

	static bool writeMetadata(std::FILE* file, const std::string& originalFilename, uint32_t originalSize);
	static bool readMetadata(std::FILE* file, std::string& originalFilename, uint32_t& originalSize);

public:
	static bool encodeFile(const std::string& inputFile, std::string& outputFile);
	static bool decodeFile(const std::string& inputFile, std::string& outputFile);
};

#endif

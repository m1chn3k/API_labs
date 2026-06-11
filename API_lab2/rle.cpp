#define _CRT_SECURE_NO_WARNINGS
#include "rle.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include <algorithm>

bool RleCoder::writeMetadata(std::FILE* file, const std::string& originalFilename, uint32_t originalSize) {
	Metadata meta;
	meta.originalSize = originalSize;
	std::memset(meta.originalFilename, 0, sizeof(meta.originalFilename));
	std::strncpy(meta.originalFilename, originalFilename.c_str(), sizeof(meta.originalFilename) - 1);

	if (std::fwrite(&meta.originalSize, sizeof(meta.originalSize), 1, file) != 1) {
		return false;
	}
	if (std::fwrite(meta.originalFilename, sizeof(meta.originalFilename), 1, file) != 1) {
		return false;
	}

	return true;
}

bool RleCoder::readMetadata(std::FILE* file, std::string& originalFilename, uint32_t& originalSize) {
	Metadata meta;
	std::memset(&meta, 0, sizeof(meta));

	if (std::fread(&meta.originalSize, sizeof(meta.originalSize), 1, file) != 1) {
		throw CorruptedMetadata("Failed to read original size");
	}

	if (std::fread(meta.originalFilename, sizeof(meta.originalFilename), 1, file) != 1) {
		throw CorruptedMetadata("Failed to read original filename");
	}

	originalSize = meta.originalSize;
	originalFilename = meta.originalFilename;

	return true;
}

bool RleCoder::encodeFile(const std::string& inputFile, std::string& outputFile) {
	std::FILE* inFile = std::fopen(inputFile.c_str(), "rb");
	if (!inFile) {
		return false;
	}

	std::fseek(inFile, 0, SEEK_END);
	uint32_t fileSize = std::ftell(inFile);
	std::fseek(inFile, 0, SEEK_SET);

	if (outputFile.empty()) {
		outputFile = inputFile + ".rle";
	}

	std::FILE* outFile = std::fopen(outputFile.c_str(), "wb");
	if (!outFile) {
		std::fclose(inFile);
		return false;
	}

	if (!writeMetadata(outFile, inputFile, fileSize)) {
		std::fclose(inFile);
		std::fclose(outFile);
		return false;
	}

	int byte = std::fgetc(inFile);

	while (byte != EOF) {
		unsigned char currentByte = static_cast<unsigned char>(byte);
		int nextByte = std::fgetc(inFile);

		if (nextByte == byte) {
			int runLength = 2;
			byte = std::fgetc(inFile);

			while (byte == nextByte && runLength < MAX_RUN_IDENTICAL) {
				runLength++;
				byte = std::fgetc(inFile);
			}

			unsigned char command = RUN_IDENTICAL_BIT | (runLength - 2);
			std::fputc(command, outFile);
			std::fputc(currentByte, outFile);
		} else {
			unsigned char diffBuffer[MAX_RUN_DIFFERENT];
			int diffCount = 1;
			diffBuffer[0] = currentByte;

			byte = nextByte;

			while (byte != EOF && diffCount < MAX_RUN_DIFFERENT) {
				diffBuffer[diffCount++] = static_cast<unsigned char>(byte);
				int peek = std::fgetc(inFile);

				if (peek == byte) {
					byte = peek;
					diffCount--;
					break;
				}

				byte = peek;
			}

			while (diffCount > 0) {
				if (diffCount >= MAX_RUN_DIFFERENT) {
					unsigned char command = MAX_RUN_DIFFERENT - 1;
					std::fputc(command, outFile);
					std::fwrite(diffBuffer, 1, MAX_RUN_DIFFERENT, outFile);

					for (int i = 0; i < diffCount - MAX_RUN_DIFFERENT; ++i) {
						diffBuffer[i] = diffBuffer[i + MAX_RUN_DIFFERENT];
					}
					diffCount -= MAX_RUN_DIFFERENT;
				} else {
					unsigned char command = diffCount - 1;
					std::fputc(command, outFile);
					std::fwrite(diffBuffer, 1, diffCount, outFile);
					diffCount = 0;
				}
			}
		}
	}

	std::fclose(inFile);
	std::fclose(outFile);
	return true;
}

bool RleCoder::decodeFile(const std::string& inputFile, std::string& outputFile) {
	std::FILE* inFile = std::fopen(inputFile.c_str(), "rb");
	if (!inFile) {
		return false;
	}

	std::string originalFilename;
	uint32_t originalSize = 0;

	try {
		readMetadata(inFile, originalFilename, originalSize);
	} catch (const CorruptedMetadata& e) {
		std::fclose(inFile);
		throw;
	}

	if (outputFile.empty()) {
		if (originalFilename.length() > 4 && originalFilename.substr(originalFilename.length() - 4) == ".rle") {
			outputFile = originalFilename.substr(0, originalFilename.length() - 4);
		} else {
			outputFile = originalFilename;
		}
		std::cout << "Suggested output file name: " << outputFile << "\n";
		std::cout << "Press Enter to accept or enter a new name: ";
		std::string userInput;
		std::getline(std::cin, userInput);
		if (!userInput.empty()) {
			outputFile = userInput;
		}
	}

	std::FILE* outFile = std::fopen(outputFile.c_str(), "wb");
	if (!outFile) {
		std::fclose(inFile);
		return false;
	}

	uint32_t decodedSize = 0;
	int command;

	try {
		while ((command = std::fgetc(inFile)) != EOF) {
			if (command & RUN_IDENTICAL_BIT) {
				int runLength = ((command & 0x7F) + 2);
				int byte = std::fgetc(inFile);
				if (byte == EOF) {
					throw InvalidRleCommand("Unexpected end of file while reading identical run");
				}
				unsigned char value = static_cast<unsigned char>(byte);

				if (decodedSize + runLength > originalSize) {
					throw InvalidRleCommand("Run length exceeds original file size");
				}

				for (int i = 0; i < runLength; ++i) {
					std::fputc(value, outFile);
					decodedSize++;
				}
			} else {
				int runLength = command + 1;
				if (decodedSize + runLength > originalSize) {
					throw InvalidRleCommand("Run length exceeds original file size");
				}

				for (int i = 0; i < runLength; ++i) {
					int byte = std::fgetc(inFile);
					if (byte == EOF) {
						throw InvalidRleCommand("Unexpected end of file while reading different run");
					}
					std::fputc(byte, outFile);
					decodedSize++;
				}
			}
		}
	} catch (const InvalidRleCommand& e) {
		std::fclose(inFile);
		std::fclose(outFile);
		throw;
	}

	if (decodedSize != originalSize) {
		std::fclose(inFile);
		std::fclose(outFile);
		throw InvalidRleCommand("Decoded size (" + std::to_string(decodedSize) + ") does not match original size (" + std::to_string(originalSize) + ")");
	}

	std::fclose(inFile);
	std::fclose(outFile);
	return true;
}

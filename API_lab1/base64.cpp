#include "base64.h"
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <algorithm>

int Base64Coder::getAlphabetIndex(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '\\') return 63;
    return -1;
}

char Base64Coder::getAlphabetChar(int index) {
    if (index < 0 || index >= ALPHABET_SIZE) return '?';
    const char* alpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+\\";
    return alpha[index];
}

std::string Base64Coder::encodeData(const std::vector<uint8_t>& data) {
    std::string result;
    int i = 0;

    while (i + 2 < data.size()) {
        uint8_t b1 = data[i++];
        uint8_t b2 = data[i++];
        uint8_t b3 = data[i++];

        int chunk1 = (b1 >> 2) & 0x3F;
        int chunk2 = ((b1 & 0x03) << 4) | ((b2 >> 4) & 0x0F);
        int chunk3 = ((b2 & 0x0F) << 2) | ((b3 >> 6) & 0x03);
        int chunk4 = b3 & 0x3F;

        result += getAlphabetChar(chunk1);
        result += getAlphabetChar(chunk2);
        result += getAlphabetChar(chunk3);
        result += getAlphabetChar(chunk4);
    }

    if (i < data.size()) {
        uint8_t b1 = data[i++];
        int chunk1 = (b1 >> 2) & 0x3F;
        int chunk2 = ((b1 & 0x03) << 4);

        result += getAlphabetChar(chunk1);
        result += getAlphabetChar(chunk2);

        if (i < data.size()) {
            uint8_t b2 = data[i];
            chunk2 = ((b1 & 0x03) << 4) | ((b2 >> 4) & 0x0F);
            int chunk3 = ((b2 & 0x0F) << 2);

            result[result.size() - 1] = getAlphabetChar(chunk2);
            result += getAlphabetChar(chunk3);
            result += PADDING;
        }
        else {
            result += PADDING;
            result += PADDING;
        }
    }

    return result;
}

std::string Base64Coder::encode(const std::vector<uint8_t>& data) {
    return encodeData(data);
}

std::string Base64Coder::encode(const char* data) {
    if (!data) return "";
    std::vector<uint8_t> bytes;
    for (const char* p = data; *p; ++p) {
        bytes.push_back(static_cast<uint8_t>(*p));
    }
    return encodeData(bytes);
}

std::string Base64Coder::encode(const std::string& data) {
    std::vector<uint8_t> bytes(data.begin(), data.end());
    return encodeData(bytes);
}

std::vector<uint8_t> Base64Coder::decode(const std::string& encoded) {
    std::vector<uint8_t> result;
    std::string cleanData;

    for (char c : encoded) {
        if (c == COMMENT_START) break;
        if (c != ' ' && c != '\n' && c != '\r' && c != '\t') {
            cleanData += c;
        }
    }

    int i = 0;
    while (i < cleanData.size()) {
        if (cleanData[i] == PADDING) break;

        if (i + 3 < cleanData.size()) {
            int c1 = getAlphabetIndex(cleanData[i]);
            int c2 = getAlphabetIndex(cleanData[i + 1]);
            int c3 = getAlphabetIndex(cleanData[i + 2]);
            int c4 = getAlphabetIndex(cleanData[i + 3]);

            if (c1 < 0 || c2 < 0 || c3 < 0 || c4 < 0) {
                i += 4;
                continue;
            }

            uint8_t b1 = ((c1 & 0x3F) << 2) | ((c2 & 0x30) >> 4);
            uint8_t b2 = ((c2 & 0x0F) << 4) | ((c3 & 0x3C) >> 2);
            uint8_t b3 = ((c3 & 0x03) << 6) | (c4 & 0x3F);

            result.push_back(b1);
            result.push_back(b2);
            result.push_back(b3);

            i += 4;
        }
        else {
            break;
        }
    }

    if (i < cleanData.size() && cleanData[i] == PADDING) {
        int numPadding = 0;
        while (i < cleanData.size() && cleanData[i] == PADDING) {
            numPadding++;
            i++;
        }

        if (numPadding == 1 && i >= 4) {
            result.pop_back();
        }
        else if (numPadding == 2 && i >= 4) {
            result.pop_back();
            result.pop_back();
        }
    }

    return result;
}

std::string Base64Coder::decodeToString(const std::string& encoded) {
    auto data = decode(encoded);
    return std::string(data.begin(), data.end());
}

std::string Base64Coder::extractOriginalFilename(const std::string& inputFile) {
    std::ifstream inFile(inputFile);
    if (!inFile.is_open()) return "";

    std::string line;
    while (std::getline(inFile, line)) {
        if (!line.empty() && line[0] == COMMENT_START) {
            if (line.find("Base64 encoded file:") != std::string::npos) {
                size_t pos = line.find("file: ") + 6;
                if (pos < line.length()) return line.substr(pos);
            }
        }
        else {
            break;
        }
    }
    inFile.close();
    return "";
}

bool Base64Coder::encodeFile(const std::string& inputFile, const std::string& outputFile) {
    std::ifstream inFile(inputFile, std::ios::binary);
    if (!inFile.is_open()) return false;

    std::vector<uint8_t> fileData((std::istreambuf_iterator<char>(inFile)),
        std::istreambuf_iterator<char>());
    inFile.close();

    std::string outFileName = outputFile.empty() ? inputFile + ".base64" : outputFile;
    std::string encodedData = encodeData(fileData);

    std::ofstream outFile(outFileName);
    if (!outFile.is_open()) return false;

    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_s(&timeinfo, &now);

    char timeBuffer[100];
    strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", &timeinfo);

    outFile << COMMENT_START << " Base64 encoded file: " << inputFile << "\n";
    outFile << COMMENT_START << " Encoded at: " << timeBuffer << "\n";
    outFile << COMMENT_START << " Size: " << fileData.size() << " bytes\n";
    outFile << COMMENT_START << "\n";

    size_t pos = 0;
    while (pos < encodedData.size()) {
        size_t lineLen = std::min(static_cast<size_t>(LINE_WIDTH), encodedData.size() - pos);
        outFile << encodedData.substr(pos, lineLen) << "\n";
        pos += lineLen;
    }

    outFile.close();
    return true;
}

bool Base64Coder::decodeFile(const std::string& inputFile, const std::string& outputFile) {
    std::ifstream inFile(inputFile);
    if (!inFile.is_open()) {
        throw DecoderException("Cannot open input file: " + inputFile);
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(inFile, line)) {
        lines.push_back(line);
    }
    inFile.close();

    // find last data line
    int lastDataLineNum = -1;
    for (int i = static_cast<int>(lines.size()) - 1; i >= 0; --i) {
        if (!lines[i].empty() && lines[i][0] != COMMENT_START) {
            lastDataLineNum = i + 1;
            break;
        }
    }

    std::string encodedData;
    bool dataEndMarkerFound = false;
    bool warningDataAfterEnd = false;

    for (size_t i = 0; i < lines.size(); ++i) {
        int currentLineNum = i + 1;
        const std::string& currentLine = lines[i];

        if (currentLine.empty() || currentLine[0] == COMMENT_START) continue;

        if (dataEndMarkerFound) {
            warningDataAfterEnd = true;
            break;
        }

        for (size_t j = 0; j < currentLine.length(); ++j) {
            if (currentLine[j] == COMMENT_START && j > 0) {
                throw IncorrectInputCharacter(currentLineNum, j + 1, currentLine[j]);
            }
        }

        for (size_t j = 0; j < currentLine.length(); ++j) {
            char c = currentLine[j];

            if (c == ' ' || c == '\t' || c == '\r') continue;

            int alphabetIndex = getAlphabetIndex(c);
            if (alphabetIndex >= 0) {
                encodedData += c;
            }
            else if (c == PADDING) {
                encodedData += c;
                for (size_t k = j + 1; k < currentLine.length(); ++k) {
                    char nextChar = currentLine[k];
                    if (nextChar != PADDING && nextChar != ' ' && nextChar != '\t' && nextChar != '\r') {
                        throw IncorrectInputCharacter(currentLineNum, k + 1, nextChar);
                    }
                }
                dataEndMarkerFound = true;
                break;
            }
            else {
                throw IncorrectInputCharacter(currentLineNum, j + 1, c);
            }
        }

        bool isLastDataLine = (currentLineNum == lastDataLineNum);
        if (currentLine.length() != LINE_WIDTH && !isLastDataLine) {
            throw IncorrectLineLength(currentLineNum, currentLine.length());
        }
    }

    if (warningDataAfterEnd) {
        std::cerr << "Warning: Data present after the end of message\n";
    }

    std::vector<uint8_t> decodedData = decode(encodedData);

    std::string outFileName = outputFile;
    if (outFileName.empty()) {
        std::string originalName = extractOriginalFilename(inputFile);
        if (!originalName.empty()) {
            std::cout << "Suggested output file name: " << originalName << "\n";
        }
        else if (inputFile.ends_with(".base64")) {
            originalName = inputFile.substr(0, inputFile.length() - 7);
            std::cout << "Suggested output file name: " << originalName << "\n";
        }
        else {
            originalName = inputFile + ".decoded";
            std::cout << "Suggested output file name: " << originalName << "\n";
        }

        std::cout << "Enter output file name (or press Enter to accept suggestion): ";
        std::getline(std::cin, outFileName);

        if (outFileName.empty()) outFileName = originalName;
    }

    std::ofstream outFile(outFileName, std::ios::binary);
    if (!outFile.is_open()) {
        throw DecoderException("Cannot create output file: " + outFileName);
    }

    outFile.write(reinterpret_cast<const char*>(decodedData.data()), decodedData.size());
    outFile.close();

    return true;
}
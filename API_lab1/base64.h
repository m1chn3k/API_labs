#ifndef BASE64_H
#define BASE64_H

#include <string>
#include <vector>
#include <stdexcept>

class DecoderException : public std::exception {
protected:
	std::string message;
public:
	DecoderException(const std::string& msg) : message(msg) {}
	const char* what() const noexcept override { return message.c_str(); }
	const std::string& getMessage() const { return message; }
};

class IncorrectInputCharacter : public DecoderException {
public:
	IncorrectInputCharacter(int line, int pos, char c) : DecoderException("") {
		std::string posStr = std::to_string(pos);
		if (posStr.length() == 1) posStr = "0" + posStr;
		message = "Line " + std::to_string(line) + ", character " + posStr + 
				  ": Incorrect input character ('" + std::string(1, c) + "')";
	}
};

class IncorrectPaddingUsage : public DecoderException {
public:
	IncorrectPaddingUsage(int line, int pos) : DecoderException("") {
		std::string posStr = std::to_string(pos);
		if (posStr.length() == 1) posStr = "0" + posStr;
		message = "Line " + std::to_string(line) + ", character " + posStr + 
				  ": Incorrect padding usage";
	}
};

class IncorrectLineLength : public DecoderException {
public:
	IncorrectLineLength(int line, int length) : DecoderException("") {
		message = "Line " + std::to_string(line) + ": Incorrect line length (" + 
				  std::to_string(length) + ")";
	}
};

class Base64Coder {
private:
	static constexpr int ALPHABET_SIZE = 64;
	static constexpr int LINE_WIDTH = 76;
	static constexpr char PADDING = '=';
	static constexpr char COMMENT_START = '-';

	static int getAlphabetIndex(char c);

	static char getAlphabetChar(int index);

	static std::string encodeData(const std::vector<uint8_t>& data);

	static std::string extractOriginalFilename(const std::string& inputFile);

public:
	static std::string encode(const std::vector<uint8_t>& data);

	static std::string encode(const char* data);

	static std::string encode(const std::string& data);

	static std::vector<uint8_t> decode(const std::string& encoded);

	static std::string decodeToString(const std::string& encoded);

	static bool encodeFile(const std::string& inputFile, const std::string& outputFile = "");

	static bool decodeFile(const std::string& inputFile, const std::string& outputFile = "");
};

#endif // BASE64_H

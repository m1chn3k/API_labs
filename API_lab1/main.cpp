#include "base64.h"
#include <iostream>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

void displayMenu() {
	std::cout << "\n=== Base64 Coder ===\n";
	std::cout << "1. Encode file\n";
	std::cout << "2. Decode file\n";
	std::cout << "3. Exit\n";
	std::cout << "Select option: ";
}

void encodeFileMenu() {
	std::cout << "\n--- Encode File ---\n";
	std::string inputFile, outputFile;

	std::cout << "Enter input file name: ";
	std::getline(std::cin, inputFile);

	if (inputFile.empty()) {
		std::cout << "Error: Input file name cannot be empty.\n";
		return;
	}

	if (!fs::exists(inputFile)) {
		std::cout << "Error: Input file does not exist.\n";
		return;
	}

	std::cout << "Enter output file name (press Enter for auto-generated name): ";
	std::getline(std::cin, outputFile);

	if (Base64Coder::encodeFile(inputFile, outputFile)) {
		std::string finalOutputFile = outputFile.empty() ? inputFile + ".base64" : outputFile;
		std::cout << "File encoded successfully.\n";
		std::cout << "Output file: " << finalOutputFile << "\n";
	} else {
		std::cout << "Error: Failed to encode file.\n";
	}
}

void decodeFileMenu() {
	std::cout << "\n--- Decode File ---\n";
	std::string inputFile, outputFile;

	std::cout << "Enter input file name (base64 file): ";
	std::getline(std::cin, inputFile);

	if (inputFile.empty()) {
		std::cout << "Error: Input file name cannot be empty.\n";
		return;
	}

	if (!fs::exists(inputFile)) {
		std::cout << "Error: Input file does not exist.\n";
		return;
	}

	std::cout << "Enter output file name (press Enter to use suggested name): ";
	std::getline(std::cin, outputFile);

	try {
		if (Base64Coder::decodeFile(inputFile, outputFile)) {
			std::cout << "File decoded successfully.\n";
		} else {
			std::cout << "Error: Failed to decode file.\n";
		}
	} catch (const IncorrectInputCharacter& e) {
		std::cerr << "Error: " << e.getMessage() << "\n";
	} catch (const IncorrectPaddingUsage& e) {
		std::cerr << "Error: " << e.getMessage() << "\n";
	} catch (const IncorrectLineLength& e) {
		std::cerr << "Error: " << e.getMessage() << "\n";
	} catch (const DecoderException& e) {
		std::cerr << "Error: " << e.getMessage() << "\n";
	}
}

int main() {
	std::cout << "Welcome to Base64 Coder\n";
	std::cout << "Custom alphabet: ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+\\\n";
	std::cout << "Padding character: '='\n";
	std::cout << "Comment marker: '-'\n";

	int choice;
	bool running = true;

	while (running) {
		displayMenu();
		std::cin >> choice;
		std::cin.ignore();

		switch (choice) {
			case 1:
				encodeFileMenu();
				break;
			case 2:
				decodeFileMenu();
				break;
			case 3:
				running = false;
				std::cout << "Goodbye!\n";
				break;
			default:
				std::cout << "Invalid option. Please try again.\n";
		}
	}

	return 0;
}

#include "CSVParser.h"
#include <sstream>
CSVParser::Tokens CSVParser::ParseLine(const std::string& line) {
	std::vector<std::string> tokens;
	std::stringstream ss(line);
	std::string raw_token;
	while (std::getline(ss, raw_token, ',')) {
		tokens.push_back(GetToken(raw_token));
	}
	return tokens;
}

CSVParser::Token CSVParser::GetToken(const std::string& rawToken) {
	size_t start = rawToken.find_first_not_of(" \t");
	if (start == std::string::npos) {
		return {};
	}
	size_t end = rawToken.find_last_not_of(" \t");
	return rawToken.substr(start, end - start + 1);
}

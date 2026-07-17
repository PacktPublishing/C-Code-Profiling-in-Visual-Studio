#pragma once
#include <string>
#include <vector>

class CSVParser
{
public:
	using Token = std::string ;
	using Tokens = std::vector<Token> ;
	[[nodiscard]]
	Tokens ParseLine(const std::string& line);
private:
	 Token GetToken(const std::string& rawToken);
};



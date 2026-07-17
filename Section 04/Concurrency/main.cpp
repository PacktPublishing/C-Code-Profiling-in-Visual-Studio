#include <vector>
#include <string>
#include <iostream>
#include <chrono>
#include <functional>
#include <filesystem>
#include <list>
#include <thread>
#include <unordered_map>
#include "CSVParser.h"
#include "LineReader.h"
void ParseFile(const LineReader& reader) {
	auto rows = reader.GetRows();
	std::clog << "[COUNT] Parsing file...\n" ;
	size_t count{};
	CSVParser parser{};
	for (const auto& row : rows) {
		auto tokens = parser.ParseLine(row);
		if (!tokens.empty() && tokens[3].find("Engineering") != std::string::npos) {
			count++;
		}
	}
	std::clog << "[COUNT] Finished\n" ;

	std::cout << "Found " << count << " employees in Engineering\n";
}


std::vector<CSVParser::Tokens> GetBulkTokens(const LineReader& reader) {
	auto bulkLines = reader.GetRows();
	std::clog << "[BULK] Processing " << bulkLines.size() << " lines\n";
	CSVParser parser{};
	std::vector<CSVParser::Tokens> bulkTokens{};
	for (const auto& line : bulkLines) {
		auto tokens = parser.ParseLine(line);
		bulkTokens.push_back(tokens);  
	}
	std::clog << "[BULK] Finishing processing\n";
	return bulkTokens;
}

std::unordered_map<std::string,int> GetManagerInfo(const LineReader &reader) {
	auto employees = reader.GetRows();
	CSVParser parser{};
	std::unordered_map<std::string,int> managers{};
	for (const auto& employeeInfo : employees) {
		auto tokens = parser.ParseLine(employeeInfo);
		if (managers.contains(tokens[4])) {
			auto &subordinateCount = managers[tokens[4]] ;
			++subordinateCount ;
			continue ;
		}
		managers.emplace(tokens[4], 1) ;
	}
	return managers;
	
}

void DisplayTokenized(std::vector<CSVParser::Tokens> bulkTokens) {
	std::cout << std::left << std::setw(8) << "ID"
		<< std::setw(22) << "Name"
		<< std::setw(12) << "Salary"
		<< std::setw(12) << "Dept"
		<< std::setw(12) << "Manager" << "\n";
	std::cout << std::string(58, '-') << "\n";  // Separator
	
	for (size_t i = 0; i < 5; ++i) {
		const auto& tokens = bulkTokens[i];
		std::cout << std::left
			<< std::setw(8) << tokens[0]    // ID
			<< std::setw(22) << tokens[1]   // Name
			<< std::setw(12) << tokens[2]   // Salary
			<< std::setw(12) << tokens[3]   // Dept
			<< std::setw(12) << tokens[4]   // Manager
			<< "\n";
	}
	std::cout << "and " << (bulkTokens.size() - 5) << " more...\n" ;
}

void DisplayManagerInfo(std::unordered_map<std::string,int> managers) {
	std::cout << '\n' ;
	std::cout << std::left 
		<< std::setw(22) << "Manager"
		<< std::setw(8) << "Subordinates"
		<< "\n";
	std::cout << std::string(40, '-') << "\n";  // Separator
	for (const auto &[manager, subordinateCount] : managers) {
		std::cout << std::left
			<< std::setw(22) << manager 
			<< ":" 
			<< std::setw(8) << subordinateCount 
			<< '\n' ;
	}
}

int main(int args, char* argv[]) try {
	LineReader reader{ argv[1] };
	reader.ReadAll();
	std::jthread t1{[&]() {
		ParseFile(reader);
	}} ;
	std::jthread t2{[&]() {
		DisplayTokenized(GetBulkTokens(reader));
	}} ;
	std::jthread t3{[&]() {
		DisplayManagerInfo(GetManagerInfo(reader)) ;
	}} ;
	

	return 0;
}
catch (const std::exception& ex) {
	std::cout << "Exception:" << ex.what() << '\n';
}

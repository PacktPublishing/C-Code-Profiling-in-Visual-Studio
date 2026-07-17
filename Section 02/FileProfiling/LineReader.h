#pragma once
#include <list>
#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>

class LineReader
{
public:
	LineReader(const std::filesystem::path &filePath);
	void ReadAll();
	auto GetRows()const {
		return m_Rows ;
	}
private:
	std::list<std::string> m_Rows{} ;
	std::ifstream m_InputStream{} ;
};



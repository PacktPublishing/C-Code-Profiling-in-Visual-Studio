#pragma once
#include <deque>
#include <list>
#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>

class LineReader
{
public:
	//using Rows = std::list<std::string>;
	using Rows = std::deque<std::string>;
	LineReader(const std::filesystem::path &filePath);
	void ReadAll();
	[[nodiscard]]
	const Rows & GetRows()const {
		return m_Rows ;
	}
private:
	Rows m_Rows{} ;
	std::ifstream m_InputStream{} ;
};



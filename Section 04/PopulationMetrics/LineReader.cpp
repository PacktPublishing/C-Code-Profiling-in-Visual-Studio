#include "LineReader.h"
LineReader::LineReader(const std::filesystem::path& filePath) {
	if (!exists(filePath)) {
		throw std::logic_error{ "File does not exist" };
	}
	m_InputStream.open(filePath) ;
	if (!m_InputStream) {
		throw std::logic_error{ "Could not open file" };
	}
}

void LineReader::ReadAll() {
	if (!m_InputStream) {
		throw std::logic_error{ "Invalid stream" };
	}
	std::clog << "[LineReader] Started\n" ;
	std::list<std::string> rows{} ;
	std::string line;
	while (std::getline(m_InputStream, line)) {
		m_Rows.push_back(line);
	}
	std::clog << "[LineReader] Finished\n" ;
	m_InputStream.close() ;
}
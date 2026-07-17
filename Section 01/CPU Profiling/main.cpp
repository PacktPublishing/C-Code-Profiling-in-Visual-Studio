#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <chrono>
#include <functional>
#include <filesystem>
#include <list>
class CSVParser {
public:
	using Token = std::string;
	using Tokens = std::vector<Token>;
	using Handler = std::function<void(const Tokens&)>;
	CSVParser() = default ;
	CSVParser(Handler handler, std::filesystem::path fileName) :
		m_Handler { std::move(handler) }, 
		m_FileName{ std::move(fileName) } {

	}
	void SetFile(std::filesystem::path fileName) {
		m_FileName = std::move(fileName);
	}
	void SetHandler(Handler handler) {
		m_Handler = std::move(handler);
	}
	void Start() {
		std::clog << "[PARSER] Working.\n" ;

		if (!exists(m_FileName)) {
			throw std::logic_error{ "File does not exist" };
		}
		std::ifstream input_stream{ m_FileName };
		if (!input_stream) {
			throw std::logic_error{ "Could not open file" };
		}
		std::string line;
		while (std::getline(input_stream, line)) {
			auto tokens = ParseLine(line);
			//auto tokens = ValidateParseLine(line);
			if (tokens.empty()) {
				std::clog << "Empty line encountered\n" ;
				continue;
			}
			if (m_Handler) {
				m_Handler(tokens);
			}
			m_Rows.push_back(tokens);
		}
		std::clog << "[PARSER] Finished.\n" ;
	}
	const std::list<Tokens>& GetRows()const {
		return m_Rows;
	}
	
private:
	Handler m_Handler{};
	std::filesystem::path m_FileName{};
	std::list<Tokens> m_Rows{};

	Token GetToken(const std::string& rawToken) {
		size_t start = rawToken.find_first_not_of(" \t");
		if (start == std::string::npos) {
			return {};
		}
		size_t end = rawToken.find_last_not_of(" \t");
		return rawToken.substr(start, end - start + 1);
	}
	Tokens ParseLine(const std::string& line) {
		Tokens tokens;
		std::stringstream ss(line);
		std::string raw_token;
		//Umar, 101,    Engineering,Mark
		while (std::getline(ss, raw_token, ',')) {
			tokens.push_back(GetToken(raw_token));
		}
		return tokens;
	}
	__declspec(noinline)
	Tokens ValidateParseLine(const std::string &line) {
		if (line.empty()) {
			std::clog << "Empty line encountered\n" ;
			return {} ;
		}
		return ParseLine(line) ;
	}
};
int main() try{
	size_t count{};
	auto callback = [&count](const CSVParser::Tokens& tokens) {
		if (tokens[3].find("Engineering") != std::string::npos) {
			count++;
		}
		return true;
		};
	CSVParser parser{ callback,"employees_100mb.csv" };
	parser.Start();
	std::cout << "Found " << count << " employees in Engineering\n";
}catch (const std::exception &ex) {
	std::cout << "Exception:" << ex.what() << '\n' ;
}

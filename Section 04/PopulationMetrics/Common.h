#pragma once
#include "CSVParser.h"
#include "LineReader.h"
std::vector<CSVParser::Tokens> GetBulkTokens_SingleThreaded(const LineReader& reader) ;
std::vector<CSVParser::Tokens> GetBulkTokens_MultiThreaded_CoarseLocking(const LineReader& reader) ;
std::vector<CSVParser::Tokens> GetBulkTokens_MultiThreaded_FineLocking(const LineReader& reader) ;
std::vector<CSVParser::Tokens> GetBulkTokens_Parallel(const LineReader& reader) ;


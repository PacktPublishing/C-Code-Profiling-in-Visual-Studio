#include "Common.h"


int main(int args, char* argv[]) try {
	LineReader reader{ argv[1] };
	reader.ReadAll();
	std::cout << "Tokenizing. Please wait.\n" ;
	//GetBulkTokens_SingleThreaded(reader);
	//GetBulkTokens_MultiThreaded_CoarseLocking(reader) ;
	//GetBulkTokens_MultiThreaded_FineLocking(reader) ;
	GetBulkTokens_Parallel(reader) ;
	
}
catch (const std::exception& ex) {
	std::cout << "Exception:" << ex.what() << '\n';
}

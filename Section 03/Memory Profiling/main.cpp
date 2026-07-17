//
//  main.cpp
//  leaksutil
//
//  Created by Umar Lone on 20/04/24.
//


#include <iostream>
#include <vector>
#include <sstream>
#include <format>
#include <numeric>

void Report(std::string_view message="Press enter to continue..."){
    std::cout << message ;
    std::cin.get() ;
    std::cout << '\n' ;
}

int GetNumberInput(std::string_view inputMessage){
    std::cout << inputMessage ;
    int input{} ;
    std::cin >> input ;
    std::cin.ignore() ;
    return input ;
}

size_t GetMemInputSize(std::string_view inputMessage){
    std::cout << inputMessage ;
    std::string input{} ;
    std::getline(std::cin, input) ;
    size_t value{} ;
    std::string unit{} ;
    std::istringstream str{input} ;
    str >> value ;
    if(value == 0){
        return {} ;
    }
    str >> unit ;
    
    constexpr size_t KB = 1024 ;
    constexpr size_t MB = KB*1024 ;
    if(unit == "kb"){
        return value * KB ;
    }
	if(unit == "mb"){
        return value * MB ;
    }
    return value ; //return bytes
}

using SizedMemory = std::pair<size_t, char*> ;
[[nodiscard]] char *Allocate(size_t size, char filler=' '){
    std::cout << "Allocating:" << size << " bytes\n" ;
    char *p = new char[size] ;
    for(size_t i = 0 ; i < size ; ++i){
        p[i] = filler ;
    }
    return p ;
}

[[nodiscard]] SizedMemory OnAllocateSizedMemory(){
    size_t bytes = GetMemInputSize("Enter size(n b,n kb,n mb) to leak(0 to exit):");
    if(bytes == 0){
        Report("No memory allocated! Press enter to continue...") ;
        return {};
    }
    char *p = Allocate(bytes, 'x') ;
    Report("Memory allocated! Press enter to continue...") ;
    return {bytes, p} ;
}

void DisplayAllocations(const std::vector<SizedMemory> &allocations){
    
    for(size_t i = 0 ; i < allocations.size() ; ++i){
        std::cout << std::format("{}. Allocated Bytes = {}\n", i, allocations[i].first) ;
    }
}

void DumpLeaks(std::vector<SizedMemory> v_chars) {
	std::cout << "\n######################\n" ;
	std::cout << "### MEMORY LEAKED! ###\n" ;
	std::cout << "######################\n" ;
	size_t index{} ;
	size_t leakCount{} ;
	for (auto memInfo : v_chars) {
		std::cout << std::format("Leak #{}: {} bytes\n", index++, memInfo.first) ;
		leakCount += memInfo.first ;
	}
	std::cout << std::format("\nTotal <{}> bytes have been leaked\n",leakCount) ;
}

void EventLoop(){
    std::vector<SizedMemory> v_chars{} ;
    size_t total_allocated_bytes{} ;
    while(true){
        system("cls") ;
        std::cout << std::format("== Total allocated bytes [{}] ==\n\n", total_allocated_bytes) ;
        std::cout << "01. Allocate Memory\n" ;
        std::cout << "02. Release memory\n" ;
        auto input = GetNumberInput("\nYour input(0 to exit)?") ;
        if(0 == input){
            break ;
        }
        switch(input){
            case 1:{
                auto mem = OnAllocateSizedMemory() ;
                if(mem.second){
                    total_allocated_bytes += mem.first ;
                    v_chars.emplace_back(mem) ;
                }
            }
                break ;
            case 2: {
	            if(v_chars.empty()){
	            	Report("No allocations found! Press enter to continue...\n") ;
	            	break ;
	            }
            	DisplayAllocations(v_chars) ;
            	auto input = GetNumberInput("Allocation to release (0 all, -1 to return)?") ;
            	if(input == -1){
            		Report("No memory released! Press enter to continue...") ;
            		break;
            	}
            	if(input >= v_chars.size()){
            		Report("Invalid index! Press enter to continue...") ;
            		break;
            	}
            	if (input == 0) {
            		for(auto mem : v_chars){
            			delete [] mem.second ;
            		}
            		v_chars.clear() ;
            		total_allocated_bytes = 0 ;
            		Report("All memory released! Press enter to continue") ;
            		break ;
            	}
            	//Else
            	auto itr = begin(v_chars) + input ;
            	total_allocated_bytes -= itr->first ;
            	delete[] itr->second ;
            	v_chars.erase(itr) ;
                
            	Report("Memory released! Press enter to continue...") ;
            	break ;
            }
        }
    }
    if(v_chars.empty()){
        std::cout << "No memory leaked!\n" ;
    }else{
        DumpLeaks(v_chars);
    }
}

int main(int argc, const char * argv[]) {
    EventLoop() ;
    Report("Press enter to exit...") ;
    return 0;
}


#include <chrono>
#include <iostream>


template<typename TimeUnit>
class Benchmark {
	std::chrono::time_point<std::chrono::high_resolution_clock> m_Start ;
	const char *m_message ;
	bool m_Done{false} ;
	constexpr const char * UnitToString()const{
		if constexpr (     std::is_same_v<TimeUnit, std::chrono::nanoseconds>) {
			return " nanoseconds";		  		  
		}								  		  
		else if constexpr (std::is_same_v<TimeUnit, std::chrono::microseconds>) {
			return " microseconds";		  		  
		}								  		  
		else if constexpr (std::is_same_v<TimeUnit, std::chrono::milliseconds>) {
			return " milliseconds";		  		  
		}								  		  
		else if constexpr (std::is_same_v<TimeUnit, std::chrono::seconds>) {
			return " seconds";			  		  
		}								  		  
		else if constexpr (std::is_same_v<TimeUnit, std::chrono::minutes>) {
			return " minutes";			  		  
		}								  		  
		else if constexpr (std::is_same_v<TimeUnit, std::chrono::hours>) {
			return " hours";
		}
		else {
			return "unknown duration type";
		}
	}
public:
	Benchmark(const char *msg=""):m_message{msg} {
		std::cout << std::format("\n>>>> Computing [{}]. Please wait <<<<\n", m_message) ;
		m_Start = std::chrono::steady_clock::now();
	}
	void Stop(){
		auto end = std::chrono::steady_clock::now() ;
		auto duration = end-m_Start ;
		if constexpr(std::is_same_v<TimeUnit, std::chrono::seconds>) {
			auto count = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()/1000.0 ;
			std::cout << std::format("\n<<<<< {} <{:.2f}> {} >>>>>\n", m_message, count, UnitToString());
		}else{
			std::cout << std::format("\n<<<<< {} <{}> {} >>>>>\n", m_message, std::chrono::duration_cast<TimeUnit>(duration).count(), UnitToString());
		}
		m_Done = true ;
	}
	~Benchmark() {
		if(!m_Done){
			Stop() ;
		}
	}
};

#define START_TIMING(message, unit) Benchmark<unit> __instance{message} 
#define STOP_TIMING __instance.Stop()

#define START_TIMING_MUL(message, unit, obj) Benchmark<unit> __instance##obj{message} 
#define STOP_TIMING_MUL(obj) __instance##obj.Stop()
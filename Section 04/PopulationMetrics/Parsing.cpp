#include <vector>
#include <string>
#include <iostream>
#include <chrono>
#include <functional>
#include <filesystem>
#include <map>
#include <mutex>
#include <thread>
#include <ranges>
#include <unordered_map>
#include <format>
#include <numeric>
#include <concepts>
#include "Common.h"
#include "p_timing.h"


void Normalize(CSVParser::Tokens& tokens) {
	for (auto& token : tokens) {
		for (auto& ch : token) {
			if (std::isalpha(ch)) {
				ch = std::toupper(ch);
			}
		}
	}
}

size_t Hash(CSVParser::Tokens& tokens) {
	size_t hashValue = 0;
	for (int i = 0; i < 4; ++i) {
		for (const auto& token : tokens) {
			hashValue ^= std::hash<std::string>{}(token)+0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);
		}
	}
	return hashValue;
}

using TimeUnit = std::chrono::seconds;

void DisplayInfo(const std::unordered_map<std::string, std::unordered_map<std::string, size_t>>& stateCityInfo) {
	constexpr std::string_view fmt = "{:<20}{:>8}{:>15}\n";
	auto title = std::format(fmt, "State", "Cities", "Population");
	std::cout << title;
	std::ranges::for_each(title, [](char) { std::cout << '='; });
	std::cout << '\n';

	for (const auto& [state, cities] : stateCityInfo) {
		auto state_population = std::accumulate(cities.begin(), cities.end(), 0ull, [](size_t sum, const std::pair<std::string, size_t>& c) {
			return sum + c.second;
			});
		std::cout << std::format(fmt, state, cities.size(), state_population);
	}
}

//Pure single threaded execution
[[nodiscard]]
std::vector<CSVParser::Tokens> GetBulkTokens_SingleThreaded(const LineReader& reader) {
	auto &rows = reader.GetRows();
	//bulkTokens will hold all tokenized lines and will be returned to caller
	std::vector<CSVParser::Tokens> bulkTokens{};
	bulkTokens.reserve(rows.size());
	START_TIMING("Singlethreaded", std::chrono::seconds) ;
	std::unordered_map<std::string, std::unordered_map<std::string, size_t>> stateCityInfo{};
	CSVParser parser{};
	for (const auto& row : rows) {
		auto tokens = parser.ParseLine(row);
		//Assuming city is at index 4 and state is at index 5
		const auto& city = tokens[4];
		const auto& state = tokens[5];
		stateCityInfo[state][city]++;

		Normalize(tokens);
		auto hash = Hash(tokens);
		//Hash is stored as first token with rest of tokens following it
		tokens.insert(tokens.begin(), std::to_string(hash));
		bulkTokens.push_back(tokens);
	}
	STOP_TIMING ;
	DisplayInfo(stateCityInfo);

	return bulkTokens;
}

const size_t WORKER_THREAD_COUNT = 20;//std::thread::hardware_concurrency();


/*
 *Common function to execute worker threads
 *Worker must be a callable with signature: void(size_t tid, size_t start, size_t end)
  where 
	tid is thread index (not id)
	start and end are the range of dataset to be processed by this thread
 */
template<typename Worker>
requires
    std::invocable<Worker, size_t, size_t, size_t> &&
    std::same_as<std::invoke_result_t<Worker, size_t, size_t, size_t>, void>
void Execute(Worker worker, const LineReader::Rows& rows) {
	//Using jthread as it will automatically join on destruction
	std::vector<std::jthread> workers{};
	//Divide dataset among available threads
	const auto workerDatasetSize = rows.size() / WORKER_THREAD_COUNT;
	for (size_t tid = 0; tid < WORKER_THREAD_COUNT; ++tid) {
		/*
		 *Computes the start and end index of the dataset for each thread. It will depend on tid
		 *and total number of threads. The last thread may have to process more data if
		 *there are remaining rows.
		 *Assume 4 threads and 10 rows:
		 *workerDatasetSize = 10 / 4 = 2
		 *The first thread will process rows [0, 2)		start=0 * 2 = 0, end=(0 == 7)? rows.size() : 0 + 2) => 2
		 *The second thread will process rows [2, 4)	start=1 * 2 = 2, end=(1 == 7)? rows.size() : 2 + 2) => 4
		 *The third thread will process rows [4, 6)		start=2 * 2 = 4, end=(2 == 7)? rows.size() : 4 + 2) => 6
		 *The fourth thread will process rows [6, 10)	start=3 * 2 = 6, end=(3 == 7)? rows.size() : 6 + 4) => 10
		 */
		auto start = tid * workerDatasetSize;
		auto end = (tid == WORKER_THREAD_COUNT - 1) ? rows.size() : start + workerDatasetSize;
		workers.emplace_back(worker, tid, start, end);
	}
}

/*
 * Multithreaded tokenizing with coarse locking. A single mutex is used to protect shared data structures.
 */
[[nodiscard]]
std::vector<CSVParser::Tokens> GetBulkTokens_MultiThreaded_CoarseLocking(const LineReader& reader) {
	std::vector<CSVParser::Tokens> bulkTokens{}; //reserve lines before hand
	auto rows = reader.GetRows();
	bulkTokens.reserve(rows.size());
	START_TIMING("MT Coarse Locking", std::chrono::seconds) ;

	std::unordered_map<std::string, std::unordered_map<std::string, size_t>> stateCityInfo{};
	std::mutex mtx;
	auto worker =
		[&rows, &bulkTokens, &mtx, &stateCityInfo](size_t tid, size_t start, size_t end) {
			//Entire processing is protected by a single lock. This defeats the purpose of multithreading
			std::lock_guard lock{ mtx };
			CSVParser parser{};
			for (; start < end; ++start) {
				auto tokens = parser.ParseLine(rows[start]);
				const auto& city = tokens[4];
				const auto& state = tokens[5];
				stateCityInfo[state][city]++;

				Normalize(tokens);
				auto hash = Hash(tokens);
				tokens.insert(tokens.begin(), std::to_string(hash));
				bulkTokens.push_back(tokens);
			}
		};
	Execute(worker, rows);
	STOP_TIMING ;

	DisplayInfo(stateCityInfo);


	return bulkTokens;
}
/*
 *Same as above but uses fine grained locking to protect shared data structures
 */
[[nodiscard]]
std::vector<CSVParser::Tokens> GetBulkTokens_MultiThreaded_FineLocking(const LineReader& reader) {
	std::vector<CSVParser::Tokens> bulkTokens{};
	auto rows = reader.GetRows();
	bulkTokens.reserve(rows.size());

	START_TIMING("MT Fine Locking", std::chrono::seconds) ;

	std::unordered_map<std::string, std::unordered_map<std::string, size_t>> stateCityInfo{};
	std::mutex mtx;
	auto worker =
		[&rows, &bulkTokens, &mtx, &stateCityInfo](size_t tid, size_t start, size_t end) {
		CSVParser parser{};
		for (; start < end; ++start) {
			auto tokens = parser.ParseLine(rows[start]);
			const auto& city = tokens[4];
			const auto& state = tokens[5];
			std::lock_guard lock{ mtx };
			/*
			 *The only data structures that are modified by multiple threads are
			 *stateCityInfo and bulkTokens. So only protect these with a mutex.
			 */
			stateCityInfo[state][city]++;

			Normalize(tokens);
			auto hash = Hash(tokens);
			tokens.insert(tokens.begin(), std::to_string(hash));
			bulkTokens.push_back(tokens);
		}
		};
	Execute(worker, rows);
	STOP_TIMING ;


	DisplayInfo(stateCityInfo);


	return bulkTokens;
}

/*
 *Uses thread local storage to avoid locking altogether, thus parallelizing the processing
 */
[[nodiscard]]
std::vector<CSVParser::Tokens> GetBulkTokens_Parallel(const LineReader& reader) {

	std::vector<CSVParser::Tokens> bulkTokens{}; //reserve lines before hand
	auto rows = reader.GetRows();
	bulkTokens.reserve(rows.size());


	//Each worker will store its results here as a separate element in the vector
	std::vector<std::vector<CSVParser::Tokens>> workerResults(WORKER_THREAD_COUNT);

	using LocalStateInfo = std::unordered_map<std::string, std::unordered_map<std::string, size_t>>;
	std::vector<LocalStateInfo> worker_state_info(WORKER_THREAD_COUNT);
	START_TIMING("Parallel Execution", std::chrono::seconds) ;

	auto worker =
		[&rows, &workerResults, &worker_state_info](size_t tid, size_t start, size_t end) {
		auto& bulkTokens = workerResults[tid];
		bulkTokens.reserve(end - start);
		CSVParser parser{};
		for (; start < end; ++start) {
			auto tokens = parser.ParseLine(rows[start]);
			const auto& city = tokens[4];
			const auto& state = tokens[5];
			/*
			 *First get the local state info for this worker thread based on tid
			 *Next update the state and city count in the local data structure.
			 *Note: This is updated to the worker specific data structure and hence
			 *does not require any locking.
			 */
			worker_state_info[tid][state][city]++;

			Normalize(tokens);
			auto hash = Hash(tokens);
			tokens.insert(tokens.begin(), std::to_string(hash));
			bulkTokens.push_back(std::move(tokens));
		}
		};


	Execute(worker, rows);
	LocalStateInfo stateCityInfo{};

	/*
	 *Now we need to merge the local state info from each worker into the global stateCityInfo
	 */
	for (auto& worker : worker_state_info) {
		for (auto& [state, cityMap] : worker) {

			auto& dstCityMap = stateCityInfo[state];

			for (auto& [city, count] : cityMap) {
				dstCityMap[city] += count;
			}
		}
	}

	/*
	 *Next, merge all the tokenized results from each worker into the final bulkTokens
	 */
	for (auto& workerResult : workerResults) {
		bulkTokens.insert(
			bulkTokens.end(),
			std::make_move_iterator(workerResult.begin()),
			std::make_move_iterator(workerResult.end()));
	}
	STOP_TIMING ;

	DisplayInfo(stateCityInfo);
	return bulkTokens;
}

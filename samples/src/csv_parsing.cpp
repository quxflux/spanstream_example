#include <array>
#include <execution>
#include <random>
#include <spanstream>
#include <sstream>

#include <benchmark/benchmark.h>

namespace
{
  template<typename InputStream>
  void parse_csv_row(const std::string_view row, std::vector<uint16_t>& contents)
  {
    std::optional<InputStream> input_stream;
    if constexpr (std::same_as<InputStream, std::istringstream>)
      input_stream = InputStream{std::string{row}};
    else if constexpr (std::same_as<InputStream, std::ispanstream>)
      input_stream = InputStream{row};

    uint16_t t{};
    while (input_stream.value() >> t)
      contents.push_back(t);
  }

  std::string generate_random_float_value_string(const size_t n, auto& rd)
  {
    std::ostringstream oss;

    for (size_t i = 0; i < n; ++i)
      oss << std::uniform_int_distribution<uint16_t>{}(rd) << " ";

    return oss.str();
  }

  template<typename StreamType>
  void benchmark_csv_parsing(benchmark::State& state)
  {
    std::mt19937 rd;

    std::optional<int64_t> last_range_size;
    std::string input_string;

    std::vector<uint16_t> parsed_data;
    parsed_data.reserve(1024);

    for (auto _ : state)
    {
      state.PauseTiming();
      parsed_data.clear();
      if (last_range_size.value_or(0) < state.range(0))
      {
        last_range_size = state.range(0);
        input_string = generate_random_float_value_string(*last_range_size, rd);
      }
      state.ResumeTiming();

      parse_csv_row<StreamType>(input_string, parsed_data);

      benchmark::DoNotOptimize(parsed_data);
      benchmark::ClobberMemory();
    }
  }

  template<typename StreamType>
  void benchmark_csv_parsing_parallel(benchmark::State& state)
  {
    std::mt19937 rd;

    std::optional<int64_t> last_range_size;
    std::string input_string;

    thread_local std::vector<uint16_t> parsed_data;
    parsed_data.reserve(1024);

    // this array controls the number of parallel tasks
    std::array<int, 64> task_indices{};
    std::ranges::iota(task_indices, 0);

    for (auto _ : state)
    {
      state.PauseTiming();
      parsed_data.clear();
      if (last_range_size.value_or(0) < state.range(0))
      {
        last_range_size = state.range(0);
        input_string = generate_random_float_value_string(*last_range_size, rd);
      }
      state.ResumeTiming();

      std::for_each(std::execution::par_unseq, task_indices.begin(), task_indices.end(), [&](const auto) {
        parse_csv_row<StreamType>(input_string, parsed_data);
        benchmark::DoNotOptimize(parsed_data);
        benchmark::ClobberMemory();
      });
    }
  }
}  // namespace

BENCHMARK(benchmark_csv_parsing<std::istringstream>)->RangeMultiplier(4)->Range(1, 512);
BENCHMARK(benchmark_csv_parsing<std::ispanstream>)->RangeMultiplier(4)->Range(1, 512);
BENCHMARK(benchmark_csv_parsing_parallel<std::istringstream>)->RangeMultiplier(4)->Range(1, 512);
BENCHMARK(benchmark_csv_parsing_parallel<std::ispanstream>)->RangeMultiplier(4)->Range(1, 512);

BENCHMARK_MAIN();

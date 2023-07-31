#include <array>
#include <cstring>
#include <execution>
#include <random>
#include <spanstream>
#include <sstream>

#include <benchmark/benchmark.h>

namespace
{
  enum class some_flag
  {
    a,
    b
  };

  constexpr std::tuple value_to_serialize{42.f, 213213, some_flag::a, some_flag::b, size_t{231}};

  template<typename F, typename Tuple>
  void for_each_tuple_element(const Tuple& t, const F& f)
  {
    [&]<size_t... Indices>(const std::index_sequence<Indices...>)
    {
      (std::invoke(f, std::get<Indices>(t)), ...);
    }
    (std::make_index_sequence<std::tuple_size_v<Tuple>>());
  }

  void write_to_ostream(auto& ostream, const auto& tuple)
  {
    for_each_tuple_element(tuple, [&]<typename TupleElement>(const TupleElement& t) {
      std::array<char, sizeof(TupleElement)> buf;
      std::memcpy(buf.data(), &t, buf.size());
      ostream.write(buf.data(), buf.size());
    });
  }

  template<typename Tuple>
  size_t serialize_to_buffer_ostringstream(const std::span<char> buffer, const Tuple& tuple)
  {
    std::ostringstream oss;
    write_to_ostream(oss, tuple);

    const auto str = oss.str();
    const auto num_bytes_to_write = static_cast<ptrdiff_t>(std::min(buffer.size(), str.size()));
    std::ranges::copy_n(str.begin(), num_bytes_to_write, buffer.begin());

    return num_bytes_to_write;
  }

  template<typename Tuple>
  size_t serialize_to_buffer_ospanstream(const std::span<char> buffer, const Tuple& tuple)
  {
    std::ospanstream oss(buffer);
    write_to_ostream(oss, tuple);

    return oss.span().size();
  }

  struct ostring_stream_serializer
  {
    template<typename Tuple>
    auto operator()(const std::span<char> buffer, const Tuple& t)
    {
      return serialize_to_buffer_ostringstream(buffer, t);
    }
  };

  struct ospanstream_serializer
  {
    template<typename Tuple>
    auto operator()(const std::span<char> buffer, const Tuple& t)
    {
      return serialize_to_buffer_ospanstream(buffer, t);
    }
  };


  template<typename T>
  void benchmark_data_serialization(benchmark::State& state)
  {
    std::vector<char> buf(128 * 1024 * 1024);
    auto buf_start = buf.begin();

    for (auto _ : state)
    {
      for (int64_t i = 0, n = state.range(0); i < n; ++i)
      {
        const auto bytes_written = T{}(std::span{buf_start, buf.end()}, value_to_serialize);
        std::advance(buf_start, bytes_written);
      }

      benchmark::DoNotOptimize(buf_start);
      benchmark::DoNotOptimize(buf);
      benchmark::ClobberMemory();
    }
  }
}  // namespace

BENCHMARK(benchmark_data_serialization<ostring_stream_serializer>)->RangeMultiplier(4)->Range(1, 512);
BENCHMARK(benchmark_data_serialization<ospanstream_serializer>)->RangeMultiplier(4)->Range(1, 512);

BENCHMARK_MAIN();

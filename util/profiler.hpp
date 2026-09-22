/*****************************************************************************

    This file is part of Modelica C Compiler.

    Modelica C Compiler is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Modelica C Compiler is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Modelica C Compiler.  If not, see <http://www.gnu.org/licenses/>.

******************************************************************************/

#ifndef MODELICACC_UTIL_PROFILER_HPP_
#define MODELICACC_UTIL_PROFILER_HPP_

#include <chrono>
#include <cstdint>
#include <iosfwd>
#include <map>
#include <string>
#include <string_view>

namespace Modelica {

using Nanoseconds = std::chrono::nanoseconds;

/**
 * @brief Statistics accumulated for one profiling label.
 */
class TimingStatistics {
public:
  std::uint64_t calls = 0;

  Nanoseconds inclusive_total{};
  Nanoseconds exclusive_total{};

  Nanoseconds inclusive_min = Nanoseconds::max();
  Nanoseconds inclusive_max{};

  Nanoseconds exclusive_min = Nanoseconds::max();
  Nanoseconds exclusive_max{};

  [[nodiscard]] double inclusive_total_ms() const noexcept;
  [[nodiscard]] double exclusive_total_ms() const noexcept;

  [[nodiscard]] double inclusive_average_ms() const noexcept;
  [[nodiscard]] double exclusive_average_ms() const noexcept;

  [[nodiscard]] double inclusive_min_ms() const noexcept;
  [[nodiscard]] double inclusive_max_ms() const noexcept;

  [[nodiscard]] double exclusive_min_ms() const noexcept;
  [[nodiscard]] double exclusive_max_ms() const noexcept;
};

/**
 * @brief Collects execution-time measurements.
 *
 * Not thread-safe.
 */
class Profiler {
public:
  using Results = std::map<std::string, TimingStatistics>;

  static Profiler& instance();

  [[nodiscard]] bool enabled() const noexcept;

  void set_enabled(bool enabled) noexcept;

  /**
   * @brief Removes all accumulated measurements.
   *
   * This function should not be called while active TimeScope objects exist.
   */
  void reset() noexcept;

  /**
   * @brief Returns the accumulated results in deterministic key order.
   */
  [[nodiscard]] const Results& results() const noexcept;

  /**
   * @brief Writes a human-readable report.
   */
  void print(std::ostream& output) const;

  /**
   * @brief Writes the results as CSV.
   */
  void write_csv(std::ostream& output) const;

  /**
   * @brief Writes the results as JSON.
   */
  void write_json(std::ostream& output) const;

private:
  Profiler() = default;

  friend class TimeScope;

  void record(
    std::string_view name,
    Nanoseconds inclusive,
    Nanoseconds exclusive
  ) noexcept;

  static double milliseconds(Nanoseconds duration) noexcept;
  static void write_json_string(std::ostream& output, std::string_view value);

  bool _enabled;
  Results _results;
};

/**
 * @brief Measures the lifetime of a scope.
 *
 * Nested scopes are supported. Inclusive time contains nested scopes while
 * exclusive time subtracts the time measured by child scopes.
 */
class TimeScope {
public:
  explicit TimeScope(std::string_view name);

  ~TimeScope() noexcept;

  TimeScope(const TimeScope&) = delete;
  TimeScope& operator=(const TimeScope&) = delete;

  TimeScope(TimeScope&&) = delete;
  TimeScope& operator=(TimeScope&&) = delete;

private:
  using Clock = std::chrono::steady_clock;

  std::string _name;
  Clock::time_point _start{};
  Nanoseconds _child_time{};
  TimeScope* _parent = nullptr;
  bool _active = false;

  static thread_local TimeScope* _current_scope;
};

/**
 * @brief Convenience function for printing the global profiler report.
 */
void time_profiler_results(std::ostream& output);

} // namespace Modelica

#endif // MODELICACC_UTIL_PROFILER_HPP_

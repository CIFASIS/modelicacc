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

#include "util/profiler.hpp"

#include <algorithm>
#include <iomanip>
#include <ostream>

namespace Modelica {

thread_local TimeScope* TimeScope::_current_scope = nullptr;

namespace {

double to_milliseconds(Nanoseconds duration) noexcept
{
  return std::chrono::duration<double, std::milli>(duration).count();
}

Nanoseconds clamp_to_zero(Nanoseconds duration) noexcept
{
  return duration < Nanoseconds::zero() ? Nanoseconds::zero() : duration;
}

} // namespace

double TimingStatistics::inclusive_total_ms() const noexcept
{
  return to_milliseconds(inclusive_total);
}

double TimingStatistics::exclusive_total_ms() const noexcept
{
  return to_milliseconds(exclusive_total);
}

double TimingStatistics::inclusive_average_ms() const noexcept
{
  if (calls == 0) {
    return 0.0;
  }

  return inclusive_total_ms() / static_cast<double>(calls);
}

double TimingStatistics::exclusive_average_ms() const noexcept
{
  if (calls == 0) {
    return 0.0;
  }

  return exclusive_total_ms() / static_cast<double>(calls);
}

double TimingStatistics::inclusive_min_ms() const noexcept
{
  if (calls == 0) {
    return 0.0;
  }

  return to_milliseconds(inclusive_min);
}

double TimingStatistics::inclusive_max_ms() const noexcept
{
  if (calls == 0) {
    return 0.0;
  }

  return to_milliseconds(inclusive_max);
}

double TimingStatistics::exclusive_min_ms() const noexcept
{
  if (calls == 0) {
    return 0.0;
  }

  return to_milliseconds(exclusive_min);
}

double TimingStatistics::exclusive_max_ms() const noexcept
{
  if (calls == 0) {
    return 0.0;
  }

  return to_milliseconds(exclusive_max);
}

Profiler& Profiler::instance()
{
  static Profiler profiler;
  return profiler;
}

void Profiler::set_enabled(bool enabled) noexcept
{
  _enabled = enabled;
}

bool Profiler::enabled() const noexcept
{
  return _enabled;
}

void Profiler::reset() noexcept
{
  _results.clear();
}

const Profiler::Results& Profiler::results() const noexcept
{
  return _results;
}

void Profiler::record(
    std::string_view name,
    Nanoseconds inclusive,
    Nanoseconds exclusive) noexcept
{
  try {
    auto& statistics = _results[std::string{name}];

    ++statistics.calls;

    statistics.inclusive_total += inclusive;
    statistics.exclusive_total += exclusive;

    statistics.inclusive_min =
        std::min(statistics.inclusive_min, inclusive);
    statistics.inclusive_max =
        std::max(statistics.inclusive_max, inclusive);

    statistics.exclusive_min =
        std::min(statistics.exclusive_min, exclusive);
    statistics.exclusive_max =
        std::max(statistics.exclusive_max, exclusive);
  }
  catch (...) {
    /*
     * Profiling must never alter the behavior of the profiled program.
     * In particular, allocation failure in the profiler is ignored.
     */
  }
}

void Profiler::print(std::ostream& output) const
{
  output << std::fixed << std::setprecision(3);

  output << "Execution time profile\n";
  output << "======================\n";

  if (_results.empty()) {
    output << "No measurements available.\n";
    return;
  }

  output
      << std::left
      << std::setw(30) << "Name"
      << std::right
      << std::setw(10) << "Calls"
      << std::setw(15) << "Total [ms]"
      << std::setw(15) << "Average [ms]"
      << std::setw(15) << "Min [ms]"
      << std::setw(15) << "Max [ms]"
      << std::setw(18) << "Exclusive [ms]"
      << '\n';

  output << std::string(120, '-') << '\n';

  for (const auto& [name, statistics] : _results) {
    output
        << std::left
        << std::setw(30) << name
        << std::right
        << std::setw(10) << statistics.calls
        << std::setw(15) << statistics.inclusive_total_ms()
        << std::setw(15) << statistics.inclusive_average_ms()
        << std::setw(15) << statistics.inclusive_min_ms()
        << std::setw(15) << statistics.inclusive_max_ms()
        << std::setw(18) << statistics.exclusive_total_ms()
        << '\n';
  }
}

void Profiler::write_csv(std::ostream& output) const
{
  output
      << "name,calls,"
      << "inclusive_total_ms,inclusive_average_ms,"
      << "inclusive_min_ms,inclusive_max_ms,"
      << "exclusive_total_ms,exclusive_average_ms,"
      << "exclusive_min_ms,exclusive_max_ms\n";

  output << std::fixed << std::setprecision(9);

  for (const auto& [name, statistics] : _results) {
    output
        << '"';

    for (const char character : name) {
      if (character == '"') {
        output << "\"\"";
      }
      else {
        output << character;
      }
    }

    output
        << "\","
        << statistics.calls << ','
        << statistics.inclusive_total_ms() << ','
        << statistics.inclusive_average_ms() << ','
        << statistics.inclusive_min_ms() << ','
        << statistics.inclusive_max_ms() << ','
        << statistics.exclusive_total_ms() << ','
        << statistics.exclusive_average_ms() << ','
        << statistics.exclusive_min_ms() << ','
        << statistics.exclusive_max_ms()
        << '\n';
  }
}

void Profiler::write_json_string(
    std::ostream& output,
    std::string_view value)
{
  output << '"';

  for (const char character : value) {
    switch (character) {
      case '"':
        output << "\\\"";
        break;
      case '\\':
        output << "\\\\";
        break;
      case '\b':
        output << "\\b";
        break;
      case '\f':
        output << "\\f";
        break;
      case '\n':
        output << "\\n";
        break;
      case '\r':
        output << "\\r";
        break;
      case '\t':
        output << "\\t";
        break;
      default:
        output << character;
        break;
    }
  }

  output << '"';
}

void Profiler::write_json(std::ostream& output) const
{
  output << std::fixed << std::setprecision(9);
  output << "{\n";
  output << "  \"measurements\": [\n";

  bool first = true;

  for (const auto& [name, statistics] : _results) {
    if (!first) {
      output << ",\n";
    }

    first = false;

    output << "    {\n";
    output << "      \"name\": ";
    write_json_string(output, name);
    output << ",\n";

    output << "      \"calls\": "
           << statistics.calls << ",\n";

    output << "      \"inclusive_total_ms\": "
           << statistics.inclusive_total_ms() << ",\n";

    output << "      \"inclusive_average_ms\": "
           << statistics.inclusive_average_ms() << ",\n";

    output << "      \"inclusive_min_ms\": "
           << statistics.inclusive_min_ms() << ",\n";

    output << "      \"inclusive_max_ms\": "
           << statistics.inclusive_max_ms() << ",\n";

    output << "      \"exclusive_total_ms\": "
           << statistics.exclusive_total_ms() << ",\n";

    output << "      \"exclusive_average_ms\": "
           << statistics.exclusive_average_ms() << ",\n";

    output << "      \"exclusive_min_ms\": "
           << statistics.exclusive_min_ms() << ",\n";

    output << "      \"exclusive_max_ms\": "
           << statistics.exclusive_max_ms() << '\n';

    output << "    }";
  }

  output << "\n  ]\n";
  output << "}\n";
}

TimeScope::TimeScope(std::string_view name)
  : _name(name)
{
  auto& profiler = Profiler::instance();

  if (!profiler.enabled()) {
    return;
  }

  _active = true;
  _parent = _current_scope;
  _current_scope = this;
  _start = Clock::now();
}

TimeScope::~TimeScope() noexcept
{
  if (!_active) {
    return;
  }

  const auto end = Clock::now();
  const auto inclusive =
      std::chrono::duration_cast<Nanoseconds>(end - _start);

  const auto exclusive =
      clamp_to_zero(inclusive - _child_time);

  /*
   * Restore the parent before recording the measurement. This keeps the
   * nesting state valid even if recording encounters an allocation failure.
   */
  _current_scope = _parent;

  if (_parent != nullptr) {
    _parent->_child_time += inclusive;
  }

  Profiler::instance().record(_name, inclusive, exclusive);
}

void time_profiler_results(std::ostream& output)
{
  Profiler::instance().print(output);
}

} // namespace Modelica

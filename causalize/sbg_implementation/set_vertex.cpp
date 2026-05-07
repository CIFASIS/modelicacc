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

#include "causalize/sbg_implementation/set_vertex.hpp"

#include <iostream>

namespace Modelica {

namespace Causalize {

// Constructors/Destructors ----------------------------------------------------

SetVertex::SetVertex(int node_id) : _node_id(node_id) {}

// Getters ---------------------------------------------------------------------

int SetVertex::node_id() const { return _node_id; }

std::size_t SetVertex::arity() const { return _starts.size(); }

std::string SetVertex::name() const { return _name; }

// Setters ---------------------------------------------------------------------

void SetVertex::set_node_id(int node_id) { _node_id = node_id; }

void SetVertex::set_name(std::string name) { _name = name; }

void SetVertex::addDimension(AST::Integer start, AST::Integer step
  , AST::Integer end)
{
  _starts.push_back(start);
  _steps.push_back(step);
  _ends.push_back(end);
}

// Operators -------------------------------------------------------------------

std::ostream& operator<<(std::ostream& out, const SetVertex& sv)
{
  out << sv.name() << ": " << sv.printSet().str();

  return out;
}

// Methods ---------------------------------------------------------------------

std::ostringstream SetVertex::printSet() const
{
  std::ostringstream out;

  unsigned int arity = _starts.size();
  for (unsigned int k = 0; k < arity; ++k) {
    out << "[" << _starts[k] << ":" << _steps[k] << ":"
      << _ends[k] << "]";
    if (k < arity - 1) {
      out << "x";
    }
  }

  return out;
}

void SetVertex::offset(AST::Integer offset)
{
  unsigned int arity = _starts.size();
  for (unsigned int k = 0; k < arity; ++k) {
    _starts[k] += offset;
    _ends[k] += offset;
  }
}

void SetVertex::concat(SetVertex other)
{
  _starts.insert(_starts.end(), other._starts.begin(), other._starts.end());
  _steps.insert(_steps.end(), other._steps.begin(), other._steps.end());
  _ends.insert(_ends.end(), other._ends.begin(), other._ends.end());
}

AST::Integer SetVertex::maxDimSize()
{
  AST::Integer maximum = 0;

  unsigned int arity = _starts.size();
  for (unsigned int k = 0; k < arity; ++k) {
    maximum = std::max(maximum, (_ends[k] - _starts[k])/_steps[k]);
  }

  return maximum;
}

} // namespace Causalize

} // namespace Modelica

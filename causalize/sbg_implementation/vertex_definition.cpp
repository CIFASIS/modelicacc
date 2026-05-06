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

#include "causalize/sbg_implementation/vertex_definition.hpp"

namespace Modelica {

namespace Causalize {

VertexDefinition::VertexDefinition(int node_id)
  : _node_id(node_id), _starts(), _steps(), _ends() {}

void VertexDefinition::addDimension(AST::Integer start, AST::Integer step
  , AST::Integer end)
{
  _starts.push_back(start);
  _steps.push_back(step);
  _ends.push_back(end);
}

std::ostringstream VertexDefinition::printSet()
{
  std::ostringstream vertex_set;

  unsigned int arity = _starts.size();
  for (unsigned int k = 0; k < arity; ++k) {
    vertex_set << "[" << _starts[k] << ":" << _steps[k] << ":"
      << _ends[k] << "]";
  }

  return vertex_set;
}

void VertexDefinition::offset(AST::Integer offset)
{
  unsigned int arity = _starts.size();
  for (unsigned int k = 0; k < arity; ++k) {
    _starts[k] += offset;
    _ends[k] += offset;
  }
}

void VertexDefinition::concat(VertexDefinition other)
{
  _starts.insert(_starts.end(), other._starts.begin(), other._starts.end());
  _steps.insert(_steps.end(), other._steps.begin(), other._steps.end());
  _ends.insert(_ends.end(), other._ends.begin(), other._ends.end());
}

AST::Integer VertexDefinition::maxDimSize()
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

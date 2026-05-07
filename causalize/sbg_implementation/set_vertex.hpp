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

#ifndef MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_SET_VERTEX_HPP_
#define MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_SET_VERTEX_HPP_

#include "ast/expression.hpp"

#include <iosfwd>
#include <sstream>
#include <string>
#include <vector>

namespace Modelica {

namespace Causalize {

/**
 * @brief Interface class between arrays of variables and equations of a model,
 * and set-vertices of the corresponding causalization SBG.
 */
class SetVertex {
public:
  SetVertex(int node_id);

  int node_id() const;
  std::size_t arity() const;
  std::string name() const;

  void set_node_id(int node_id);
  void set_name(std::string name);
  void addDimension(AST::Integer start, AST::Integer step, AST::Integer end);

  /**
   * @brief Prints the corresponding array in SBG program format.
   */
  std::ostringstream printSet() const;

  void offset(AST::Integer offset);
  void concat(SetVertex other);

  /**
   * @brief Returns the maximum size between dimensions. That is, if the array
   * was declared: Type x[n1, ..., nk]; it returns max{ni : 0 < i < k+1}.
   */
  AST::Integer maxDimSize();

private:
  int _node_id;
  std::string _name;
  std::vector<AST::Integer> _starts;
  std::vector<AST::Integer> _steps;
  std::vector<AST::Integer> _ends;
};

std::ostream& operator<<(std::ostream& out, const SetVertex& sv);

} // namespace Causalize

} // namespace Modelica

#endif // MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_SET_VERTEX_HPP_

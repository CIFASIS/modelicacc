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

#include "causalize/sbg_implementation/model_match.hpp"
#include "causalize/sbg_implementation/ast_visitors/eq_var_match_annotator.hpp"
#include "util/debug.hpp"

namespace Modelica {

namespace Causalize {

////////////////////////////////////////////////////////////////////////////////
// ModelicaCC variable-equation matching ---------------------------------------
////////////////////////////////////////////////////////////////////////////////

EqVarMatch::EqVarMatch(AST::Equation equation, AST::ExpList variables)
  : _equation(equation), _variables(variables) {}

const AST::Equation& EqVarMatch::equation() const { return _equation; }

const AST::ExpList& EqVarMatch::variables() const { return _variables; }

std::ostream& operator<<(std::ostream& out, const EqVarMatch& match)
{
  EqVarMatchAnnotator annotator{match.variables()};
  out << Apply(annotator, match.equation()).str();
  return out;
}

////////////////////////////////////////////////////////////////////////////////
// ModelicaCC matching of a model ----------------------------------------------
////////////////////////////////////////////////////////////////////////////////

std::size_t ModelMatch::size() const { return _model_match.size(); }

EqVarMatch ModelMatch::operator[](std::size_t k) const
{
  ERROR_UNLESS(k < _model_match.size(), "ModelMatch::operator[]: index ", k
    , " out of range");
  return _model_match[k];
}

void ModelMatch::pushBack(EqVarMatch match) { _model_match.push_back(match); }

void ModelMatch::concatenation(ModelMatch other)
{
  _model_match.insert(
    _model_match.end(), other._model_match.begin(), other._model_match.end()
  );
}

std::ostream& operator<<(std::ostream& out, const ModelMatch& match)
{
  for (std::size_t k = 0; k < match.size(); ++k) {
    out << match[k] << "\n";
  }

  return out;
}

}  // namespace Causalize

}  // namespace Modelica

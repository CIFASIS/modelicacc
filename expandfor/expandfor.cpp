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

#include <expandfor/expandfor.h>
#include <ast/equation.h>
#include <ast/queries.h>
#include <boost/variant/get.hpp>
#include <util/ast_visitors/expandfor_visitor.h>
#include <causalize/state_variables_finder.h>
#include <algorithm>
#include <vector>


namespace Modelica {
    
    ExpandFor::ExpandFor(MMO_Class &c): _c(c) {
    }
    
    void ExpandFor::expandFor() {
        EquationList &el = _c.equations_ref().equations_ref();
        ExpandForVisitor efv;
        foreach_(Equation &e, el) {
            e = boost::apply_visitor(efv, e);
        }
    }

};


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

#include <util/ast_visitors/to_micro/to_micro_st.h>
#include <ast/queries.h>
#include <stdio.h>
#include <boost/variant/apply_visitor.hpp>
#define apply(X) boost::apply_visitor(*this,X)

namespace Modelica {

    using namespace boost;
    toMicroSt::toMicroSt(MMO_Class &cl, unsigned int &discont): mmo_class(cl), disc_count(discont) {
    };
    Statement toMicroSt::operator()(Assign v) const { 
      return v;
    }

    Statement toMicroSt::operator()(Break v) const { 
      return v;
    }

    Statement toMicroSt::operator()(Return v) const { 
      return v;
    }

    Statement toMicroSt::operator()(CallSt v) const { 
      return v;
    }

    Statement toMicroSt::operator()(IfSt v) const { 
      return v;
    }

    Statement toMicroSt::operator()(ForSt v) const { 
      return v;
    }

    Statement toMicroSt::operator()(WhenSt v) const { 
      return v;
    }

    Statement toMicroSt::operator()(WhileSt v) const { 
      return v;
    }

    StatementList toMicroSt::statements() const { return statements_; }
}

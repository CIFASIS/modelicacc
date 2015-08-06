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


#ifndef AST_EXPRESSION
#define AST_EXPRESSION
#include <ast/ast_types.h>
#include <string>
#include <iostream>
#include <vector>


namespace Modelica {
  namespace AST {

    using namespace boost;
    
    extern const char *BinOpTypeName[];
    typedef int             Integer;
    typedef double          Real;
    typedef std::string     Name;
    typedef enum { Or, And, Lower, LowerEq, Greater, GreaterEq, CompNotEq, CompEq, Add, ElAdd, Sub, ElSub, Div, ElDiv, Mult, ElMult, Exp, ElExp } BinOpType ;
    typedef enum { Not, Minus, Plus } UnaryOpType ;
    typedef enum { FALSE, TRUE } Bool;

    struct BinOp;
    struct UnaryOp;
    struct Boolean;
    struct IfExp;
    struct String ;
    struct SubEnd;
    struct SubAll;
    struct Call;
    struct Brace;
    struct Bracket;
    struct FunctionExp;
    struct ForExp;
    struct Reference;
    struct Range;
    struct Output;
    struct Named;


    typedef variant<
      Integer,
      String,
      Boolean,
      Name,
      Real,
      SubEnd,
      SubAll,
      recursive_wrapper<BinOp>,
      recursive_wrapper<UnaryOp>,
      recursive_wrapper<Brace>,
      recursive_wrapper<Bracket>,
      recursive_wrapper<Call>,
      recursive_wrapper<FunctionExp>,
      recursive_wrapper<ForExp>,
      recursive_wrapper<IfExp>,
      recursive_wrapper<Named>,
      recursive_wrapper<Output>,
      recursive_wrapper<Reference>,
      recursive_wrapper<Range>
    > Expression;

    typedef std::vector<Expression> ExpList;
    typedef std::vector<ExpList> ExpListList;
    typedef Option<Expression> OptExp;
    typedef std::vector<OptExp> OptExpList;
    typedef Pair<Name, ExpList > RefTuple;
    typedef Pair<Expression, Expression> ExpPair;
    typedef std::vector<RefTuple> Ref;
    typedef std::vector<ExpPair> ElseIf;

    struct SubEnd {
      comparable(SubEnd);
      printable(SubEnd);
    };
    
    struct SubAll {
      comparable(SubAll);
      printable(SubAll);
    };

    struct String  {
      String(std::string s);
      String(std::vector<boost::optional<char> > s);
      String();
      friend std::ostream& operator<<(std::ostream& out, const String &s);
      comparable(String);
      member(std::string, val);
    };

    typedef std::vector<String> StringList;
    typedef std::vector<Name> IdentList;

    struct Boolean {
      Boolean (){};
      Boolean (Bool b);
    
      printable(Boolean);
      comparable(Boolean);
      member(bool, val);
    };

    struct BinOp {
      BinOp(){};
      BinOp(Expression l, BinOpType op, Expression r);

      comparable(BinOp);
      printable(BinOp);
      member(Expression,left);
      member(Expression,right);
      member(BinOpType,op);
    };

    struct IfExp {
      IfExp ();
      IfExp (Expression a, Expression b, List<ExpPair> elseif, Expression c);
      IfExp (Expression a, Expression b, std::vector<fusion::vector2<Expression, Expression> > elseif, Expression c);
      comparable(IfExp);
      printable(IfExp);
      member(Expression,cond);
      member(Expression,then);
      member(List<ExpPair>, elseif)
      member(Expression,elseexp);
    };

    struct Call {
      Call(){};
      Call(Name n, Expression arg);
      Call(Name n, ExpList args);
      printable(Call);
      comparable(Call);
      member(Name,name);
      member(ExpList,args)
    };
    
    struct ForExp {
      ForExp(){};
      ForExp(Expression e, ExpList indices);
      comparable(ForExp);
      printable(ForExp);
      member(Expression,exp);
      member(ExpList, indices);
    };


    struct Named {
      Named() {};
      Named(Name n, Expression e);
      printable(Named);
      comparable(Named);
      member(Name,name);
      member(Expression,exp);
    };

    struct Index {
      Index (){};
      Index (Name n, OptExp e);
      printable(Index);
      comparable(Index);
      member(OptExp,exp)
      member(Name,name)
    };

    typedef std::vector<Index> IndexList;

    struct Indexes {
      Indexes (){};
      Indexes (IndexList ind):indexes_(ind) {};
      printable(Indexes);
      comparable(Indexes);
      member(IndexList, indexes);
    };

    struct UnaryOp {
      UnaryOp (){};
      UnaryOp (Expression e, UnaryOpType op);
      printable(UnaryOp);
      comparable(UnaryOp);
      member(Expression,exp);
      member(UnaryOpType,op);
    };

    struct Reference {
      Reference(){};
      Reference(Option<Name> dot, Name n, Option<ExpList> i);
      Reference(Reference, Name n, Option<ExpList> i);
      Reference(Name n);
      Reference(Ref r);
      printable(Reference); 
      comparable(Reference);
      member(Ref,ref);
    };
 
    struct Range {
      Range (){};
      Range (Expression s, Expression e);
      Range (Expression s, Expression i, Expression e);
      comparable(Range);
      printable(Range);
      member(Expression,start);
      member(OptExp,step);
      member(Expression,end);
    };

    struct Output {
      Output (){};
      Output (Expression e);
      Output (OptExpList l);
      printable(Output);
      comparable(Output);
      member(OptExpList,args)
    };

    struct FunctionExp {
      FunctionExp (){};
      FunctionExp (Name n, ExpList args);
      printable(FunctionExp);
      comparable(FunctionExp);
      member(Name,name)
      member(ExpList,args)
    };
 
    struct Brace {
      Brace(){};
      Brace(Expression arg);
      Brace(ExpList args);
      printable(Brace);
      comparable(Brace);
      member(ExpList,args) 
    };

    struct Bracket {
      Bracket(){};
      Bracket(ExpListList args);
      printable(Bracket);
      comparable(Bracket);
      member(ExpListList,args)
    };

    extern Boolean True, False;
    
    template<typename T> 
    inline bool is(Expression e) {
      return e.type()==typeid(T);
    }
  };
};
#endif

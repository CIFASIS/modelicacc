
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

#include <flatter/remove_composition.h>
#include <boost/variant/get.hpp>

static int label;

Name nextIndexLabel()
{
  stringstream ret(stringstream::out);
  ret << "_Index_" << label++;
  return ret.str();
}

Remove_Composition::Remove_Composition() {}

void Remove_Composition::LevelUp(MMO_Class &up, MMO_Class &down, Name nUp, VarInfo viUp)
{
  DotExpression _dot = DotExpression(Option<MMO_Class &>(down), nUp, ExpList());
  foreach_(Name n, down.variables())
  {
    Option<VarInfo> viDown = down.syms_ref()[n];
    if (viDown) {
      Name tDown = viDown.get().type();
      Name tUp = viUp.type();
      Name newName = nUp + "_" + n;
      ExpList iexp;
      Option<ExpList> ind;
      if (viUp.indices()) iexp += viUp.indices().get();
      if (viDown.get().indices()) iexp += viDown.get().indices().get();
      if (iexp.size() > 0) ind = iexp;
      Option<Modification> opt_mod = viDown.get().modification();
      if (opt_mod && viUp.indices()) {  // adjust for arrays
        Modification &mod = opt_mod.get();
        if (is<ModEq>(mod)) {
          // get<ModEq>(mod).exp_ref() = Call("fill",ExpList(2,get<ModEq>(mod).exp(),iexp.front()));
          ExpList args(1, get<ModEq>(mod).exp());
          BOOST_FOREACH (Expression e, iexp)
            args.push_back(e);
          get<ModEq>(mod).exp_ref() = Call("fill", args);
        } else if (is<ModClass>(mod)) {
          ModClass &mc = get<ModClass>(mod);
          foreach_(Argument & arg, mc.modification_ref())
          {
            if (is<ElMod>(arg)) {
              ElMod &elm = get<ElMod>(arg);
              if ("start" == elm.name() || "fixed" == elm.name()) elm.each_ref() = true;
            }
          }
        }
      }
      if (opt_mod) {
        Modification &mod = opt_mod.get();
        if (is<ModEq>(mod)) {
          get<ModEq>(mod).exp_ref() = Apply(_dot, get<ModEq>(mod).exp_ref());
        } else if (is<ModClass>(mod)) {
          ModClass &mc = get<ModClass>(mod);
          if (mc.exp()) mc.exp_ref() = Apply(_dot, mc.exp().get());
        }
      }

      VarInfo v = VarInfo(viDown.get().prefixes(), tDown, viDown.get().comment(), opt_mod, ind, false);
      v.removePrefix(output);
      v.removePrefix(input);
      up.syms_ref().insert(newName, v);
      up.variables_ref().push_back(newName);
    }
  }

  foreach_(Name n, down.types())
  {
    Option<Type::Type> t = down.tyTable_ref()[n];
    if (t) {
      up.types_ref().push_back(n);
      up.tyTable_ref().insert(n, t.get());
    }
  }

  // MODIFICACIONES A LAS ECUACIONES Y LOS STATEMENTS

  if (!viUp.indices()) {
    DotExpression dot = DotExpression(Option<MMO_Class &>(down), nUp, ExpList());
    EqDotExpression eqChange = EqDotExpression(dot);
    StDotExpression stChange = StDotExpression(dot);

    foreach_(Equation eq, down.equations_ref().equations()) up.addEquation(Apply(eqChange, eq));
    foreach_(Equation eq, down.initial_eqs_ref().equations()) up.addInitEquation(Apply(eqChange, eq));

    foreach_(Statement st, down.statements_ref().statements()) up.addStatement(Apply(stChange, st));
    foreach_(Statement st, down.initial_sts_ref().statements()) up.addInitStatement(Apply(stChange, st));
  } else {
    ExpList index = viUp.indices().get();
    ExpList indexVar;
    IdentList indexString;

    foreach_(Expression e, index)
    {
      Name n = nextIndexLabel();
      indexVar.push_back(Reference(Reference(), n, Option<ExpList>()));
      indexString.push_back(n);
    }

    DotExpression dot = DotExpression(Option<MMO_Class &>(down), nUp, indexVar);
    EqDotExpression eqChange = EqDotExpression(dot);
    StDotExpression stChange = StDotExpression(dot);

    EquationList eqs = down.equations_ref().equations();
    EquationList eqsI = down.initial_eqs_ref().equations();
    StatementList sts = down.statements_ref().statements();
    StatementList stsI = down.initial_sts_ref().statements();

    foreach_(Equation & eq, eqs) eq = Apply(eqChange, eq);
    foreach_(Equation & eq, eqsI) eq = Apply(eqChange, eq);

    foreach_(Statement & st, sts) st = Apply(stChange, st);
    foreach_(Statement & st, stsI) st = Apply(stChange, st);

    if (eqs.size() > 0) up.addEquation(createForEquation(indexString, index, eqs));
    if (eqsI.size() > 0) up.addInitEquation(createForEquation(indexString, index, eqsI));
    if (sts.size() > 0) up.addStatement(createForStatement(indexString, index, sts));
    if (stsI.size() > 0) up.addInitStatement(createForStatement(indexString, index, stsI));
  }
}

Equation Remove_Composition::createForEquation(IdentList index, ExpList indexVar, EquationList el)
{
  int i = 0;
  IndexList ilist;
  foreach_(Name it, index)
  {
    ilist.push_back(Index(it, OptExp(Range(1, indexVar[i]))));
    i++;
  }
  return ForEq(Indexes(ilist), el);
}

Statement Remove_Composition::createForStatement(IdentList index, ExpList indexVar, StatementList el)
{
  int i = 0;
  IndexList ilist;
  foreach_(Name it, index)
  {
    ilist.push_back(Index(it, OptExp(Range(1, indexVar[i]))));
    i++;
  }
  return ForSt(Indexes(ilist), el);
}

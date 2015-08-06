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

#include <util/loadlib/loadlib.h>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/variant/get.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <util/debug.h>
#include <parser/parser.h>

using namespace std;
using namespace boost::filesystem;
using namespace boost;
namespace Modelica
{
	using namespace Modelica::AST;
	Option<ClassType> loadClass(Name n) {
    char_separator<char> sep(".");
    tokenizer< char_separator<char> > tokens(n, sep);
    string base="/usr/lib/omlibrary/Modelica 3.2.1/";
    foreach_(const string& t, tokens) {
	    if (t=="Modelica") continue;
  	  path p(base+t);
    	if (exists(p) && is_directory(p)) {
	    	base+=t+"/";
    	} else { 
	    	path pp(base+t+".mo");
		    if (exists(pp) && is_regular_file(pp)) {
          bool ret;
          string file = base+t+".mo";
          StoredDef sd = parseFile(file,ret);
          if (!ret) {
            WARNING("Could not read file %s\n", file.c_str());
          } else {
            string prefix("");
            if (sd.within() && sd.name()) 
              prefix=sd.name().get();
            foreach_(ClassType c, sd.classes()) {
              Name cl = prefix + "." + className(c);
              if (n==cl) {
                return c;
              }
              if (starts_with(n,cl) && is<Class>(c)) {
                Class &rc = boost::get<Class>(c);
                prefix += "." + className(c);
                bool cont=true;
                while (cont) {
                  cont = false;
                  foreach_(Element e, rc.composition().elements()) {
                    if (is<ElemClass>(e)) {
                      ElemClass elc = boost::get<ElemClass>(e);
                      ClassType clt = elc.class_element().get().cl();
                      file = prefix + "." + className(clt);
                      if (file==n) {
                        // TODO: Resolve within and change name??
                        return clt;
                      } else {
                        // TODO: Continue downwards
                      }
                       
  
                    }

                  }
                }
              }
            }

          }
  			  break;	
  	  	}
  	  }
    }
	  return Option<ClassType> ();
	}
};

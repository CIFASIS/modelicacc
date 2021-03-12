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
#ifndef GRAPH_PRINTER_
#define GRAPH_PRINTER_

#include<util/graph/graph_definition.h>

using namespace std;

namespace SBG {
  class GraphPrinter{
    public:
    GraphPrinter(const SBGraph &g, const int mod);

    void printGraph(std::string name);

    private:
    int depth;
    const SBGraph &graph; 
    const int mode;

    void printVertices(stringstream &stri);
    void printEdges(stringstream &stri);
    std::string vPrinter(SetVertex v);
    std::string vLabelPrinter(SetVertex v);
    std::string ePrinter(SetEdge e);
  };
} // namespace SBG

#endif

/*****************************************************************************
 
    This file is part of the Set-Based Graph library.

******************************************************************************/

#pragma once

#include <util/graph/sbg/sbg.h>

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

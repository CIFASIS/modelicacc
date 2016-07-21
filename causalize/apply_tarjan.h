/*
 * apply_tarjan.h
 *
 *  Created on: 21/12/2014
 *      Author: fede
 */

#ifndef APPLY_TARJAN_H_
#define APPLY_TARJAN_H_

#include <causalize/graph/graph_definition.h>
#include <utility>
#include <list>
#include <map>

namespace Causalize {
  struct Component {
    std::list<Vertex> *uVertices;
    std::list<Vertex> *eqVertices;
  };
  typedef Component *ComponentPtr;
  int apply_tarjan(CausalizationGraph &graph, std::map<int, ComponentPtr> &components);
}


#endif /* APPLY_TARJAN_H_ */

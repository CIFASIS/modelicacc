Estructura del Algoritmo:

* 

	bool steps = 0;
	bool split = false; // Inicialmente split va a ser false. Excepto que no se pueda matchear ninguna ecuación entera y vamos a volver 
											//   a probar todo intentando con split = true. Luego de matchear, reiniciaremos split a false.

	map< < pair <EqVertex, MDI> , UVertex > yaAsignado; 

	while (true){
		bool causalize_some=false;
		assert(equationNumber == unknownNumber);
    if(equationDescriptors.empty() && unknownDescriptors.empty()) {
      // Finished causalizing :)
      if (debugIsEnabled('c'))
        PrintCausalizationResult();
      if (solve) // @karupayun: assert(solve())?
        SolveEquations();
      return true;
    }
		
    list<VectorVertex>::iterator iter, auxiliaryIter;


    //We are only trying to process the equationside, because in this case is the same.
		auxiliaryIter = equationDescriptors.begin();
    for(iter = auxiliaryIter; iter != equationDescriptors.end(); iter = auxiliaryIter){ // Ecuación
      // Additional iterator to erase while traversing
      auxiliaryIter++;
      EquationVertex eq = *iter;
			/*------------------------------*/
      ERROR_UNLESS(out_degree(eq, graph) != 0, "Problem is singular, not supported yet\n"); 
      // Try to look for a set of indexes to causalize
      Option<std::pair<VectorEdge,IndexPairSet> > op = CanCausalize(eq, kVertexEquation, split); // Acá busca causalizar 
      // If we can causalize something
      if (op) {
        // We are going to causalize something
        causalize_some=true;
        split = false;
        // This pair holds which edge(the first component) to use for causalization and which indexes(the second component)
        std::pair<VectorEdge,IndexPairSet> causal_pair = op.get();
        ERROR_UNLESS(causal_pair.second.size()==1, "Causalizing more than a singleton");
        VectorEdge e = causal_pair.first;
        // This is the unknown node connecting to the edge
        UnknownVertex unk = GetUnknown(e);
        equationNumber--;
        unknownNumber--;
        // Save the result of this step of causalization
        Causalize1toN(graph[unk].unknown, graph[eq].equation, causal_pair.second);
        // Update the pairs in the edge that is being causalized
        if (debugIsEnabled('c')) 
          std::cerr << "Causalizing from the equation side " << causal_pair.second << std::endl;
        graph[e].RemovePairs(causal_pair.second);
        // Decrement the number of uncausalized equations/unknowns
        graph[eq].count -= causal_pair.second.begin()->Dom().Size();
        graph[unk].count -= causal_pair.second.begin()->Ran().Size();
        // If the edge has no more pairs in it remove it
        if (graph[e].IsEmpty()) {
          if (debugIsEnabled('c')) {
            std::cout << "Removing the edge\n";
          }
          remove_edge(e, graph);
        }
        // Auxiliary list to later remove empty edges
        std::list<VectorEdge> remove;
        foreach_(VectorEdge e1, out_edges(unk,graph)) {
          // Update the labels from all the edges adjacent to the unknown
          if (debugIsEnabled('c')) {
            std::cout << "Removing unknowns " << causal_pair.second.begin()->Ran() << " from " << graph[e1]<<"\n";
          }
          graph[e1].RemoveUnknowns(causal_pair.second.begin()->Ran());
          if (debugIsEnabled('c')) {
            std::cout << "Result: " << graph[e1] << "\n";
          }
          // If the edge is now empty schedule it for removal
          if (graph[e1].IsEmpty()) {
            if (debugIsEnabled('c')) {
              std::cout << "Removing the edge\n";
            }
            remove.push_back(e1);
          }
        }
        // Now remove all scheduled edges
        foreach_(VectorEdge e1, remove) {
          WARNING_UNLESS(out_degree(GetEquation(e1),graph)>1, "Disconnecting equation node");
          remove_edge(e1, graph);
        }
        // If the equation node is now unconnected and with count==0 we can remove it
        if (out_degree(eq,graph)==0) {
          ERROR_UNLESS(graph[eq].count==0, "Disconnected node with uncausalized equations");
          remove_vertex(eq,graph);
          equationDescriptors.erase(iter);
        }
        // If the unknown node is now unconnected and with count==0 we can remove it
        if (out_degree(unk,graph)==0) {
          ERROR_UNLESS(graph[unk].count==0, "Disconnected node with uncausalized unknowns");
          remove_vertex(unk,graph);
          unknownDescriptors.remove(unk);
        }
        /*stringstream ss;
        ss << "graph_" << step++ << ".dot";
        GraphPrinter<VectorVertexProperty,Label>  gp(graph);
        gp.printGraph(ss.str());*/
      }
    }		
	
}

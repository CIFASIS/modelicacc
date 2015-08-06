#include <boost/test/unit_test.hpp>
#include <boost/test/included/unit_test.hpp>

#include <causalize/apply_tarjan.h>

#include <util/debug.h>

using namespace boost::unit_test;

//____________________________________________________________________________//

void apply_tarjan_test()
{
    CausalizationGraph graph;

    VertexProperties vp1;
    vp1.type = E;
    Vertex e1 = add_vertex(vp1, graph);
    VertexProperties vp2;
    vp2.type = U;
    Vertex u1 = add_vertex(vp2, graph);
    VertexProperties vp3;
    vp3.type = E;
    Vertex e2 = add_vertex(vp3, graph);
    VertexProperties vp4;
    vp4.type = U;
    Vertex u2 = add_vertex(vp4, graph);
    VertexProperties vp5;
    vp5.type = E;
    Vertex e3 = add_vertex(vp5, graph);
    VertexProperties vp6;
    vp6.type = U;
    Vertex u3 = add_vertex(vp6, graph);
    VertexProperties vp7;
    vp7.type = E;
    Vertex e4 = add_vertex(vp7, graph);
    VertexProperties vp8;
    vp8.type = U;
    Vertex u4 = add_vertex(vp8, graph);
    VertexProperties vp9;
    vp9.type = E;
    Vertex e5 = add_vertex(vp9, graph);
    VertexProperties vp10;
    vp10.type = U;
    Vertex u5 = add_vertex(vp10, graph);

    add_edge(e1, u3, graph);
    add_edge(e1, u4, graph);
    add_edge(e2, u2, graph);
    add_edge(e3, u2, graph);
    add_edge(e4, u1, graph);
    add_edge(e4, u2, graph);
    add_edge(e5, u1, graph);
    // The following edges  form a cycle.
    add_edge(e3, u3, graph);
    add_edge(e3, u5, graph);
    add_edge(e5, u3, graph);
    add_edge(e5, u5, graph);


    std::map<int, causalize::Component> components;

    BOOST_CHECK(apply_tarjan(graph, components) == 4);

}

//____________________________________________________________________________//

test_suite*
init_unit_test_suite( int, char* [] ) {

    debugInit("c");

    framework::master_test_suite().p_name.value = "Apply Tarjan";

    framework::master_test_suite().add( BOOST_TEST_CASE( &apply_tarjan_test ));

    return 0;
}


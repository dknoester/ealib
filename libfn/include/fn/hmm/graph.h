#ifndef _FN_HMM_GRAPHVIZ_H_
#define _FN_HMM_GRAPHVIZ_H_

#include <boost/graph/adjacency_list.hpp>
#include <iostream>
#include <string>
#include <fn/hmm/hmm_network.h>

namespace fn {
    namespace hmm {
        
        struct vertex_properties {
            enum node_type { NONE, INPUT, OUTPUT, HIDDEN, GATE };
            
            vertex_properties() : nt(NONE), idx(0), node(0) {
            }

            node_type nt;
            int idx;
            fn::hmm::hmm_node* node;
        };
        
        struct edge_properties {
        };
                
        typedef boost::adjacency_list<boost::setS,
            boost::vecS,
            boost::bidirectionalS,
            vertex_properties,
            edge_properties> hmm_graph;
        
        struct reduced_edge {
            reduced_edge(hmm_graph& g) : _g(g) {
            }
            
            bool operator()(hmm_graph::edge_descriptor e);
            
            hmm_graph& _g;
        };

        template <typename VertexDescriptor, typename Graph>
        bool has_edges(VertexDescriptor u, const Graph& g) {
            return (boost::in_degree(u,g) > 0) || (boost::out_degree(u,g) > 0);
        }
        
        hmm_graph as_genetic_graph(hmm_network& h);
        hmm_graph as_reduced_graph(hmm_network& h);
        hmm_graph as_causal_graph(hmm_network& h);

        void write_graphviz(const std::string& title, std::ostream& out, const hmm_graph& g, bool detailed=false);
        
    } // hmm
} // fn


#endif

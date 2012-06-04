#ifndef _EA_ANALYSIS_GIRVAN_NEWMAN_CLUSTERING_H_
#define _EA_ANALYSIS_GIRVAN_NEWMAN_CLUSTERING_H_

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/bc_clustering.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <vector>

namespace ea {
    namespace analysis {
        
        //! Always returns true, used to exhaust edges for clustering.
        template <typename T, typename EdgeList>
        struct exhaust_edges {
            typedef T centrality_type;
            EdgeList& removed_edges;
            
            exhaust_edges(EdgeList& el) : removed_edges(el) { }
            
            template<typename Graph, typename Edge>
            bool operator()(T max_centrality, Edge e, const Graph& g) {
                removed_edges.push_back(e);
                return false;
            }
        };
        
        
        /*! Girvan-Newman "clustering" (community structure).
         
         Returns a list of edges (vertex pairs) in the order that they were removed
         from the graph.
         */
        template <typename Graph>//, typename ModuleMap>
        std::vector<typename Graph::edge_descriptor> girvan_newman_clustering(Graph g) {
            typedef typename Graph::vertex_descriptor Vertex;
            typedef typename Graph::edge_descriptor Edge;
            typedef std::vector<Edge> edge_list;

            typedef std::map<Edge, int> StdEdgeIndexMap;
            StdEdgeIndexMap my_e_index;
            typedef boost::associative_property_map< StdEdgeIndexMap > EdgeIndexMap;
            EdgeIndexMap e_index(my_e_index);
            
            int i=0;
            typename Graph::edge_iterator ei,ei_end;
            for(boost::tie(ei,ei_end)=boost::edges(g); ei!=ei_end; ++ei) {
                my_e_index.insert(std::make_pair(*ei,i++));
            }
            
            std::vector<double> e_centrality_vec(boost::num_edges(g), 0.0);
            boost::iterator_property_map<std::vector<double>::iterator, EdgeIndexMap> e_centrality_map(e_centrality_vec.begin(), e_index);

            edge_list removed_edges;
            exhaust_edges<double, edge_list> terminate(removed_edges);
            
            betweenness_centrality_clustering(g, terminate, e_centrality_map );
            
            return removed_edges;
        }
    } // analysis
} // ea

#endif

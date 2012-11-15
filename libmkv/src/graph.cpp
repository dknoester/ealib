#include <bitset>
#include <set>
#include <boost/graph/reverse_graph.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <mkv/graph.h>


mkv::markov_graph mkv::as_genetic_graph(markov_network& net) {
    
    markov_graph g(net.nstates() + net.ngates());
    
    // color the states:
    int v=0; // vertex counter
    for(std::size_t i=0; i<net.ninput_states(); ++i, ++v) {
        g[boost::vertex(v,g)].nt = vertex_properties::INPUT;
        g[boost::vertex(v,g)].idx = v;
    }
    for(std::size_t i=0; i<net.noutput_states(); ++i, ++v) {
        g[boost::vertex(v,g)].nt = vertex_properties::OUTPUT;
        g[boost::vertex(v,g)].idx = v;
    }
    for(std::size_t i=0; i<net.nhidden_states(); ++i, ++v) {
        g[boost::vertex(v,g)].nt = vertex_properties::HIDDEN;
        g[boost::vertex(v,g)].idx = v;
    }
    
    // add the edges:
    for(std::size_t i=0; i<net.ngates(); ++i, ++v) {
        g[boost::vertex(v,g)].nt = vertex_properties::GATE;
        g[boost::vertex(v,g)].idx = v;
        boost::apply_visitor(detail::graph_building_visitor(boost::vertex(v,g), g), net[i]);
    }
    
    return g;
}


bool mkv::reduced_edge::operator()(markov_graph::edge_descriptor e) {
    // if the edge targets a node that is an input, remove.
    if(_g[boost::target(e,_g)].nt == vertex_properties::INPUT) {
        return true;
    }
    
    // if the edge sources a node that is not an input, and that node is otherwise unconnected, remove.
    if((_g[boost::source(e,_g)].nt != vertex_properties::INPUT)
       && (boost::in_degree(boost::source(e,_g),_g) == 0)) {
        return true;
    }    
    
    // if the edge targets a node that is not an output, and that node is otherwise unconnected, remove.
    if((_g[boost::target(e,_g)].nt != vertex_properties::OUTPUT)
       && (boost::out_degree(boost::target(e,_g),_g) == 0)) {
        return true;
    }    
    
    return false;
}


template <typename NodeSet>
struct node_recorder : public boost::default_bfs_visitor {
    node_recorder(NodeSet& ns) : visited(ns) {
    }
    
    template <typename Vertex, typename Graph>
    void discover_vertex(Vertex u, const Graph & g) const {
        visited.insert(u);
    }
    
    NodeSet& visited;
};


/*! Produce a reduced version of the hmm network.
 */
mkv::markov_graph mkv::as_reduced_graph(markov_network& h) {
    markov_graph g = as_genetic_graph(h);
    
    std::size_t last_edges;
    do {
        last_edges = boost::num_edges(g);
        boost::remove_edge_if(reduced_edge(g), g);
    } while(boost::num_edges(g) != last_edges);
    
    boost::reverse_graph<markov_graph> r(g);
    std::set<markov_graph::vertex_descriptor> ns;
    node_recorder<std::set<markov_graph::vertex_descriptor> > nr(ns); 
    
    // yes, iterate over the vertices in the original graph, 
    // and if they're outputs, bfs in the reversed graph:
    markov_graph::vertex_iterator vi,vi_end;
    for(boost::tie(vi,vi_end)=boost::vertices(g); vi!=vi_end; ++vi) {
        if(g[*vi].nt == vertex_properties::OUTPUT) {
            boost::breadth_first_search(r, *vi, boost::visitor(nr));
        }
    }
    
    // now, get rid of everyone that we *didn't visit*.
    // two stages to avoid problems with iterator invalidation.
    std::vector<markov_graph::vertex_descriptor> rmlist;
    for(boost::tie(vi,vi_end)=boost::vertices(g); vi!=vi_end; ++vi) {
        if(nr.visited.find(*vi) == nr.visited.end()) {
            rmlist.push_back(*vi);
        }
    }
    
    // just remove the edges; it's cleaned up in printing.
    for(std::vector<markov_graph::vertex_descriptor>::iterator i=rmlist.begin(); i!=rmlist.end(); ++i) {
        boost::clear_vertex(*i,g);
    }
    
    return g;
}


/*! Produce a causal version of the markov network.
 */
mkv::markov_graph mkv::as_causal_graph(markov_network& h) {
    markov_graph g = as_reduced_graph(h);
    
    markov_graph::vertex_iterator vi,vi_end;
    
    for(boost::tie(vi,vi_end)=boost::vertices(g); vi!=vi_end; ++vi) {
        if((boost::in_degree(*vi,g) > 0) || (boost::out_degree(*vi,g) > 0)) {
            if(g[*vi].nt == vertex_properties::GATE) {
                markov_graph::out_edge_iterator oei,oei_end;
                for(boost::tie(oei,oei_end)=boost::out_edges(*vi,g); oei!=oei_end; ++oei) {
                    
                    markov_graph::in_edge_iterator iei,iei_end;
                    for(boost::tie(iei,iei_end)=boost::in_edges(*vi,g); iei!=iei_end; ++iei) {
                        
                        boost::add_edge(boost::source(*iei,g),
                                        boost::target(*oei,g),
                                        g);
                    }
                }
                boost::clear_vertex(*vi,g);
            }
        }
    }
    
    return g;
}


void mkv::write_graphviz(const std::string& title, std::ostream& out, const markov_graph& g) {
    
    out << "digraph {" << std::endl << "edge [ arrowsize=0.75 ];" << std::endl;
    out << "labelloc=\"t\"" << std::endl << "label=\"" << title << "\"" << std::endl;
    
    markov_graph::vertex_iterator vi,vi_end;
    
    out << "subgraph {" << std::endl << "rank=same;" << std::endl;
    for(boost::tie(vi,vi_end)=boost::vertices(g); vi!=vi_end; ++vi) {
        if(has_edges(*vi,g) && (g[*vi].nt == vertex_properties::INPUT)) {
            out << g[*vi].idx << " [color=green];" << std::endl;
        }
    }
    out << "}" << std::endl;
    
    out << "subgraph {" << std::endl << "rank=same;" << std::endl;
    for(boost::tie(vi,vi_end)=boost::vertices(g); vi!=vi_end; ++vi) {
        if(has_edges(*vi,g) && (g[*vi].nt == vertex_properties::OUTPUT)) {
            out << g[*vi].idx << " [color=red];" << std::endl;
        }
    }
    out << "}" << std::endl;
    
    out << "subgraph {" << std::endl << "rank=same;" << std::endl;
    for(boost::tie(vi,vi_end)=boost::vertices(g); vi!=vi_end; ++vi) {
        if(has_edges(*vi,g) && (g[*vi].nt == vertex_properties::HIDDEN)) {
            out << g[*vi].idx << " [color=blue];" << std::endl;
        }
    }
    out << "}" << std::endl;
    
    std::string nl="A";
    out << "subgraph {" << std::endl << "rank=same;" << std::endl;
    for(boost::tie(vi,vi_end)=boost::vertices(g); vi!=vi_end; ++vi) {
        if(has_edges(*vi,g) && (g[*vi].nt == vertex_properties::GATE)) {
            out << g[*vi].idx << " ";
            out << "[shape=box,label=\"" << nl << "\"];" << std::endl;
            if(*nl.rbegin() == 'Z') {
                std::string::reverse_iterator i=nl.rbegin();
                for( ; i!=nl.rend(); ++i) {
                    if(*i == 'Z') {
                        *i = 'A';
                    } else {
                        ++*i;
                        break;
                    }
                }
                if(i == nl.rend()) {
                    nl.push_back('A');
                }
            }
        }
    }
    out << "}" << std::endl;
    
    markov_graph::edge_iterator ei,ei_end;
    for(boost::tie(ei,ei_end)=boost::edges(g); ei!=ei_end; ++ei) {
        out << g[boost::source(*ei,g)].idx << "->" << g[boost::target(*ei,g)].idx;
        switch(g[*ei].et) {
            case edge_properties::REINFORCE: {
                out << " [color=green];" << std::endl;
                break;
            }
            case edge_properties::INHIBIT: {
                out << " [color=red];" << std::endl;
                break;
            }
            default: {
                out << ";" << std::endl;
            }
        }
    }
    
    out << "}" << std::endl;
}

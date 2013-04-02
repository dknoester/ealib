/* epistasis.h 
 * 
 * This file is part of EALib.
 * 
 * Copyright 2012 David B. Knoester.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _EA_ANALYSIS_EPISTASIS_H_
#define _EA_ANALYSIS_EPISTASIS_H_

#include <boost/graph/adjacency_list.hpp>
#include <iostream>
#include <string>
#include <cmath>
#include <ea/datafile.h>
#include <ea/exceptions.h>

#include <ea/analysis/graph.h>
#include <ea/analysis/individual.h>
#include <ea/analysis/modularity.h>
#include <ea/analysis/tool.h>

namespace ealib {
    namespace analysis {
        
        //! Epistasis edges.
        struct epistasis_edge_properties {
            epistasis_edge_properties() : weight(0.0) { }
            epistasis_edge_properties(double w) : weight(w) { }
            double weight;
        };
        
        //! Epistasis vertices.
        struct epistasis_vertex_properties {
            epistasis_vertex_properties() : color(0), bgcolor(0) { }
            int color;
            int bgcolor;
            std::string annotation;
        };

        typedef boost::adjacency_list<boost::vecS, // allows parallel edges
        boost::vecS,
        boost::undirectedS,
        epistasis_vertex_properties,
        epistasis_edge_properties> epistasis_graph;
        
        
        /*! Calculates the epistasis map for the given individual.
         
         This is done by generating all the double mutants within radius r of each loci
         and calculating the epistasis of those mutants compared to the wildtype.
         
         \returns an epistasis graph that connects interacting loci by edges weighted by their epistasis.
         Edge weights < 1e-6 are pruned (assumed to be fp errors).
         */
        template <typename EA>
        epistasis_graph epistasis(const typename EA::individual_type& indi, std::size_t r, EA& ea) {
            check_argument(r>0, "radius of double mutations must be > 0");
            
            typedef typename EA::fitness_type ftype;
            typedef typename EA::representation_type::codon_type ctype;
            
            // get a copy:
            typename EA::individual_type ind(indi);
            typename EA::representation_type& repr=representation(ind,ea);
            
            epistasis_graph g(repr.size());
            ftype w_0 = recalculate_fitness(ind,ea); // wildtype fitness
            
            for(std::size_t i=0; i<repr.size(); ++i) {
                r = repr[i] >> 1;
                // i == site of first mutation
                for(std::size_t j=(i+1); j<(i+r+1); ++j) {
                    // j == site of second mutation
                    ctype a=repr[i], b=repr[j]; // current values of sites a and b

                    repr[i] = a ^ 0x01;
                    ftype w_a = recalculate_fitness(ind,ea); // single mutant a
                    
                    repr[j] = b ^ 0x01;
                    ftype w_ab = recalculate_fitness(ind,ea); // double mutant ab
                    
                    repr[i] = a;
                    ftype w_b = recalculate_fitness(ind,ea); // single mutant b
                    
                    repr[j] = b; // undo b!
                    
                    double e = log((w_0*w_ab)/(w_a*w_b));
                    if(fabs(e) > get<EPISTASIS_THRESHOLD>(ea)) {
                        boost::add_edge(boost::vertex(i,g),
                                        boost::vertex(j%repr.size(),g), // mod in order to take circularity into account
                                        epistasis_edge_properties(e),
                                        g);
                    }
                }
            }
            
            return g;
        }
        
        
        template <typename EA>
        void epistasis(EA& ea) {
            typedef typename EA::fitness_type ftype;
            typedef typename EA::representation_type::codon_type ctype;
            
            // get a copy:
            typename EA::individual_type ind=find_most_fit_individual(ea);
            epistasis_graph g = epistasis(ind, get<NK_MODEL_K>(ea), ea);
            ealib::datafile df("epistasis.dot");
            write_circular_graph(g, df);
        }
        // LIBEA_ANALYSIS_TOOL(epistasis, "calculate epistasis among all loci in a genome")
        
        
        template <typename EA>
        void individual_epistatic_modularity(EA& ea) {
            typename EA::individual_type indi=individual_load(get<ANALYSIS_INPUT>(ea), ea);

            // calculate its epistasis graph:
            epistasis_graph g = epistasis(indi, get<NK_MODEL_K>(ea), ea);
            
            // normalize the epistasis graph to prepare it for module calculation:
            graph_normalize(g);
            // write_circular_graph(g, datafile(get<ANALYSIS_OUTPUT>(ea) + "/epi", c, ".dot"));
            
            // calculate its modules:
            modularity_result m = modularity(g);

            // write the annotated modular graph to a file:
            write_colored_circular_graph(m.g, datafile(get<ANALYSIS_OUTPUT>(ea) + "/epi-mod.dot"));
        }
        // LIBEA_ANALYSIS_TOOL(individual_epistatic_modularity, "calculate epistasis among all loci in a genome")

        
        template <typename EA>
        void epistatic_modularity(EA& ea) {
            datafile qdf("modularity.dat");
            qdf.add_field("individual [individual]")
            .add_field("max QN [maxqn]")
            .add_field("edges removed [removed]")
            .add_field("edges remaining [remaining]");
            
            datafile qn("qn.dat");
            
            // for each genome in the population:
            int c=0;
            for(typename EA::population_type::iterator i=ea.population().begin(); i!=ea.population().end(); ++i, ++c) {
                // calculate its epistasis graph:
                epistasis_graph g = epistasis(ind(i,ea), get<NK_MODEL_K>(ea), ea);
                
                write_circular_graph(g, datafile(get<ANALYSIS_OUTPUT>(ea) + "/epi", c, ".dot"));
                
                // normalize the epistasis graph to prepare it for module calculation:
                graph_normalize(g);
                // write_circular_graph(g, datafile(get<ANALYSIS_OUTPUT>(ea) + "/epi", c, ".dot"));
                
                // calculate its modules:
                modularity_result m = modularity(g);
                
                // write info about the modular graph:
                qdf.write(c).write(m.max_q).write(m.removed).write(boost::num_edges(m.g)).endl();
                
                // write the annotated modular graph to a file:
                write_colored_circular_graph(m.g, datafile(get<ANALYSIS_OUTPUT>(ea) + "/epi-mod", c, ".dot"));
                
                // write info about all the rejected modular graphs:
                for( int j=0; j<(get<NK_MODEL_N>(ea) * get<NK_MODEL_K>(ea)); ++j) {
                    if(j < m.qn.size()) {
                        qn.write(m.qn[j]);
                    } else {
                        qn.write(0.0);
                    }
                }
                qn.endl();
            }
        }
        // LIBEA_ANALYSIS_TOOL(epistatic_modularity, "calculate epistatic modularity for all genomes in a population");
        
        
    } // analysis
} // ea


#endif

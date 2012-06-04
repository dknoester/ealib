#ifndef _EA_ANALYSIS_LINE_OF_DESCENT_H_
#define _EA_ANALYSIS_LINE_OF_DESCENT_H_

#include <iostream>
#include <sstream>
#include <cmath>
#include <vector>
#include <ea/datafile.h>
#include <ea/interface.h>
#include <ea/line_of_descent.h>
#include <ea/analysis/tool.h>
#include <ea/analysis/graph.h>
#include <ea/analysis/girvan_newman_clustering.h>
#include <ea/analysis/modularity.h>


namespace ea {
    namespace analysis {
        
        /*! Produce a datafile describing the origination time, fixation time, and
         size (number of loci) of mutational events along the LoD.
         */
        template <typename EA>
        struct lod_fixation_times : public ea::analysis::unary_function<EA> {
            static const char* name() { return "lod_fixation_times"; }
            
            virtual void operator()(EA& ea) {
                using namespace ea;
                using namespace ea::analysis;
                using namespace boost::accumulators;
                
                line_of_descent<EA> lod = lod_load(get<ANALYSIS_INPUT>(ea), ea);
                lod.runiq();
                
                datafile df(get<ANALYSIS_OUTPUT>(ea));
                df.add_field("mutation orgination time [origination]")
                .add_field("mutation fixation time [fixation]")
                .add_field("mutation size [size]");
                
                typename line_of_descent<EA>::iterator k=lod.begin(); ++k; // ancestor
                typename line_of_descent<EA>::iterator j=k; ++j; // parent
                typename line_of_descent<EA>::iterator i=j; ++i; // offspring
                
                for(; i!=lod.end(); ++i, ++j, ++k) {
                    typename EA::representation_type& ir=ind(*i,ea).repr();
                    typename EA::representation_type& jr=ind(*j,ea).repr();
                    typename EA::representation_type& kr=ind(*k,ea).repr();
                    
                    unsigned int tally=0;
                    for(std::size_t x=0; x<ir.size(); ++x) {
                        if((ir[x] != jr[x]) && (ir[x] != kr[x])) {
                            ++tally;
                        }                    
                    }
                    
                    df.write(ind(i,ea).update()).write(get<FIXATION_TIME>(ind(i,ea))).write(tally).endl();
                }
            }
        };

        /*! Calculate the epistatic modularity along the line of descent.
         
         What we're doing here is calculating epistasis and modularity for each
         genome along the line of descent.
         */
        template <typename EA>
        struct lod_epistasis : public ea::analysis::unary_function<EA> {
            static const char* name() { return "lod_epistasis"; }

            virtual void operator()(EA& ea) {
                using namespace ea;
                using namespace ea::analysis;
                using namespace boost::accumulators;
                
                line_of_descent<EA> lod = lod_load(get<ANALYSIS_INPUT>(ea), ea);
                lod.uniq();
                
                datafile df("epistasis.dat");
                df.add_field("generation [generation]")
                .add_field("lod depth [depth]")
                .add_field("num modules [modules]")
                .add_field("mean degree [mean_degree]")
                .add_field("newman modularity [qn]")
                .add_field("hintze modularity [qh]")
                .add_field("epistasis [epi]");
                
                // for each genome along the line of descent...
                int c=0;
                typename line_of_descent<EA>::reverse_iterator i=lod.rbegin();
                ++i; // skip ancestor
                for( ; i!=lod.end(); ++i, ++c) {
                    // calculate its epistasis graph:
                    epistasis_graph g = ea.fitness_function().epistasis(ind(*i,ea), ea);
                    
                    // take the abs of the epistasis graph to prepare it for module calculation:
                    graph_abs(g);
                    
                    accumulator_set<double, stats<tag::mean> > degree;
                    epistasis_graph::vertex_iterator vi,vi_end;
                    for(boost::tie(vi,vi_end)=boost::vertices(g); vi!=vi_end; ++vi) {
                        degree(boost::degree(*vi,g));
                    }
                    
                    // color the components of g
                    int nc = color_components(g);
                    if(exists<ANALYSIS_OUTPUT>(ea)) {
                        write_colored_circular_graph(g, datafile(get<ANALYSIS_OUTPUT>(ea) + "/epistasis", c, ".dot"));
                    }
                    
                    df.write(ind(*i,ea).generation())
                    .write(c)
                    .write(nc)
                    .write(mean(degree))
                    .write(newman_modularity(g))
                    .write(hintze_modularity(g))
                    .write(graph_sum(g)).endl();
                }
            }
        };
        
    } // analysis
} // ea

#endif

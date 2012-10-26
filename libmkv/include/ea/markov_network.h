/* markov_network.h
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
#ifndef _EA_MARKOV_NETWORK_H_
#define _EA_MARKOV_NETWORK_H_

#include <boost/algorithm/string/predicate.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/max.hpp>

#include <ea/interface.h>
#include <ea/meta_data.h>
#include <ea/analysis.h>
#include <ea/events.h>
#include <ea/datafile.h>

#include <mkv/markov_network.h>

// meta-data
LIBEA_MD_DECL(MKV_INPUT_N, "markov_network.input.n", int);
LIBEA_MD_DECL(MKV_OUTPUT_N, "markov_network.output.n", int);
LIBEA_MD_DECL(MKV_HIDDEN_N, "markov_network.hidden.n", int);
LIBEA_MD_DECL(MKV_UPDATE_N, "markov_network.update.n", int);
LIBEA_MD_DECL(MKV_GATE_TYPES, "markov_network.gate_types", std::string);
LIBEA_MD_DECL(MKV_INITIAL_GATES, "markov_network.initial_gates", int);
LIBEA_MD_DECL(MKV_REPR_INITIAL_SIZE, "markov_network.representation.initial_size", unsigned int);
LIBEA_MD_DECL(MKV_REPR_MAX_SIZE, "markov_network.representation.max_size", unsigned int);
LIBEA_MD_DECL(MKV_REPR_MIN_SIZE, "markov_network.representation.min_size", unsigned int);
LIBEA_MD_DECL(GATE_INPUT_LIMIT, "markov_network.gate.input.limit", int);
LIBEA_MD_DECL(GATE_INPUT_FLOOR, "markov_network.gate.input.floor", int);
LIBEA_MD_DECL(GATE_OUTPUT_LIMIT, "markov_network.gate.output.limit", int);
LIBEA_MD_DECL(GATE_OUTPUT_FLOOR, "markov_network.gate.output.floor", int);


namespace mkv {
    
    enum gate_types { MARKOV=42, LOGIC=43 };
    
    /*! Returns a set of supported gate types.
     */
    template <typename EA>
    std::set<gate_types> supported_gates(EA& ea) {
        using namespace ea;
        std::set<gate_types> supported;
        std::string gates = get<MKV_GATE_TYPES>(ea);
        if(boost::icontains(gates,"markov")) { supported.insert(MARKOV); }
        if(boost::icontains(gates,"logic")) { supported.insert(LOGIC); }
        return supported;
    }
}

namespace ea {
	
	/*! Markov network mutation type.
     
     Performs per-site, duplication, and deletion mutations.
	 */
	struct mkv_mutation {
		template <typename Representation, typename EA>
		void operator()(Representation& repr, EA& ea) {
			typedef typename std::vector<typename Representation::codon_type> codon_buffer;
			
			double per_site_p = get<MUTATION_PER_SITE_P>(ea);
            int imax = get<MUTATION_UNIFORM_INT_MAX>(ea);
			for(typename Representation::iterator i=repr.begin(); i!=repr.end(); ++i) {
				if(ea.rng().p(per_site_p)) {
					*i = ea.rng()(imax);
				}
			}
			
            // gene duplication
            // (the below looks a little crude, but there were some problems related
            // to incorrect compiler optimization.)
			if(ea.rng().p(get<MUTATION_DUPLICATION_P>(ea)) && (repr.size()<get<MKV_REPR_MAX_SIZE>(ea))) {
                int start = ea.rng().uniform_integer(0, repr.size());
                int extent = ea.rng().uniform_integer(16, 513);
                codon_buffer buf(extent);
                for(int i=0; i<extent; ++i) {
                    buf[i] = repr[start+i];
                }
				repr.insert(ea.rng().choice(repr.begin(),repr.end()), buf.begin(), buf.end());
			}
            
            // gene deletion
			if(ea.rng().p(get<MUTATION_DELETION_P>(ea)) && (repr.size()>get<MKV_REPR_MIN_SIZE>(ea))) {
				int start, extent;
				extent = 15+ea.rng()(512);
				start = ea.rng()(repr.size()-extent);
				repr.erase(repr.begin()+start, repr.begin()+start+extent);
			}
		}
	};
    
    
    /*! Generates random Markov network-based individuals.
	 */
	struct mkv_random_individual {
        template <typename EA>
        typename EA::representation_type operator()(EA& ea) {
            using namespace mkv;
            
            typename EA::representation_type repr;
            repr.resize(get<MKV_REPR_INITIAL_SIZE>(ea), 127);
			
            // which gate types are supported?
            std::set<gate_types> supported = supported_gates(ea);
            
			int i,j;
			for(i=0; i<get<MKV_INITIAL_GATES>(ea); ++i) {
				j=ea.rng()(repr.size()-100);
                int gate=*ea.rng().choice(supported.begin(), supported.end()); //ea.rng()(3);
                repr[j] = gate;
                repr[j+1] = 255-gate;
				for(int k=2; k<97; ++k) {
					repr[j+k]=ea.rng()(256);
				}
			}
			return repr;
		}
	};
}

namespace mkv {
    namespace detail {
        
        /*! Build a markov gate.
         */
        template <typename Network, typename ForwardIterator, typename MetaData>
        void build_io(Network& net, index_list_type& inputs, index_list_type& outputs, ForwardIterator& h, MetaData& md) {
            using namespace ea;
            using namespace ea::algorithm;

            int nin=modnorm(*h++, get<GATE_INPUT_FLOOR>(md), get<GATE_INPUT_LIMIT>(md));
            int nout=modnorm(*h++, get<GATE_OUTPUT_FLOOR>(md), get<GATE_OUTPUT_LIMIT>(md));

            inputs.clear();
            inputs.insert(inputs.end(), h, h+nin);
            std::transform(inputs.begin(), inputs.end(), inputs.begin(), std::bind2nd(std::modulus<int>(), net.nstates()));
            h+=nin;

            outputs.clear();
            outputs.insert(outputs.end(), h, h+nout);
            std::transform(outputs.begin(), outputs.end(), outputs.begin(), std::bind2nd(std::modulus<int>(), net.nstates()));
            h+=nout;
        }
        
        
        /*! Build a markov gate.
         */
        template <typename Network, typename ForwardIterator, typename MetaData>
        void build_markov_gate(Network& net, ForwardIterator h, MetaData& md) {
            index_list_type inputs, outputs;
            build_io(net, inputs, outputs, h, md);
            markov_gate g(inputs, outputs, h);
            net.append(g);
        }
        
        
        /*! Build a logic gate.
         */
        template <typename Network, typename ForwardIterator, typename MetaData>
        void build_logic_gate(Network& net, ForwardIterator h, MetaData& md) {
            index_list_type inputs, outputs;
            build_io(net, inputs, outputs, h, md);
            logic_gate g(inputs, outputs, h);
            net.append(g);
        }
    }
    
    /*! Build a Markov network from the genome [f,l), with the given meta data.
     */
    template <typename ForwardIterator, typename MetaData>
    void build_markov_network(markov_network& net, ForwardIterator f, ForwardIterator l, MetaData& md) {
        using namespace detail;
        using namespace ea;
        using namespace ea::algorithm;
        using namespace mkv;
        
        if(f == l) { return; }
        
        // which gate types are supported?
        std::set<gate_types> supported = supported_gates(md);

        ForwardIterator last=f;
        ++f;
        
        for( ; f!=l; ++f, ++last) {
            int start_codon = *f + *last;
            if(start_codon == 255) {
                switch(*last) {
                    case MARKOV: { // build a markov gate
                        if(!supported.count(MARKOV)) { break; }
                        build_markov_gate(net, f+1, md);
                        break;
                    }
                    case LOGIC: { // build a logic gate
                        if(!supported.count(LOGIC)) { break; }
                        build_logic_gate(net, f+1, md);
                        break;
                    }
                    default: {
                        break;
                    }
                }
            }
        }
    }
        
//    /*! Save the dominant individual in graphviz format.
//     */
//    template <typename EA>
//    struct mkv_genetic_graph : public ea::analysis::unary_function<EA> {
//        static const char* name() { return "mkv_genetic_graph"; }
//        
//        virtual void operator()(EA& ea) {
//            using namespace ea;
//            using namespace ea::analysis;
//            typename EA::individual_type ind = analysis::find_most_fit_individual(ea);
//            
//            markov_network net(get<MKV_INPUT_N>(ea), get<MKV_OUTPUT_N>(ea), get<MKV_HIDDEN_N>(ea), ea.rng());
//            build_markov_network(net, ind.repr().begin(), ind.repr().end(), ea);
//            
//            datafile df(get<ANALYSIS_OUTPUT>(ea));
//            std::ostringstream title;
//            //            title << "individual=" << i.name() << "; generation=" << i.generation() << "; fitness=" << static_cast<std::string>(i.fitness());
//            mkv::write_graphviz(title.str(), df, mkv::as_genetic_graph(net));
//        }
//    };
//    
//    
//    /*! Save the dominant individual in graphviz format.
//     */
//    template <typename EA>
//    struct mkv_meta_population_reduced_graph : public ea::analysis::unary_function<EA> {
//        static const char* name() { return "mkv_meta_population_reduced_graph"; }
//        
//        virtual void operator()(EA& ea) {
//            using namespace ea;
//            using namespace ea::analysis;
//            
//            int count=0;
//            for(typename EA::iterator i=ea.begin(); i!=ea.end(); ++i) {
//                typename EA::individual_type::individual_type& ind = analysis::find_most_fit_individual(*i);
//                markov_network net(get<MKV_INPUT_N>(ea), get<MKV_OUTPUT_N>(ea), get<MKV_HIDDEN_N>(ea), ea.rng());
//                build_markov_network(net, ind.repr().begin(), ind.repr().end(), ea);
//                
//                datafile df("reduced_sp" + boost::lexical_cast<std::string>(count++) + ".dot");
//                std::ostringstream title;
//                mkv::write_graphviz(title.str(), df, mkv::as_reduced_graph(net));
//            }
//        }
//    };
//    
//    
//    /*! Save the dominant individual in graphviz format.
//     */
//    template <typename EA>
//    struct mkv_reduced_graph : public ea::analysis::unary_function<EA> {
//        static const char* name() { return "mkv_reduced_graph"; }
//        
//        virtual void operator()(EA& ea) {
//            using namespace ea;
//            using namespace ea::analysis;
//            typename EA::individual_type ind = analysis::find_most_fit_individual(ea);
//            markov_network net(get<MKV_INPUT_N>(ea), get<MKV_OUTPUT_N>(ea), get<MKV_HIDDEN_N>(ea), ea.rng());
//            build_markov_network(net, ind.repr().begin(), ind.repr().end(), ea);
//            datafile df(get<ANALYSIS_OUTPUT>(ea));
//            std::ostringstream title;
//            mkv::write_graphviz(title.str(), df, mkv::as_reduced_graph(net), false);
//        }
//    };
//    
//    
//    /*! Save the detailed graph of the dominant individual in graphviz format.
//     */
//    template <typename EA>
//    struct mkv_detailed_graph : public ea::analysis::unary_function<EA> {
//        static const char* name() { return "mkv_detailed_graph"; }
//        
//        virtual void operator()(EA& ea) {
//            using namespace ea;
//            using namespace ea::analysis;
//            typename EA::individual_type ind = analysis::find_most_fit_individual(ea);
//            markov_network net(get<MKV_INPUT_N>(ea), get<MKV_OUTPUT_N>(ea), get<MKV_HIDDEN_N>(ea), ea.rng());
//            build_markov_network(net, ind.repr().begin(), ind.repr().end(), ea);
//            datafile df(get<ANALYSIS_OUTPUT>(ea));
//            std::ostringstream title;
//            //            title << "individual=" << i.name() << "; generation=" << i.generation() << "; fitness=" << static_cast<std::string>(i.fitness());
//            mkv::write_graphviz(title.str(), df, mkv::as_reduced_graph(net), true);
//        }
//    };
//    
//    
//    /*! Save the causal graph of the dominant individual in graphviz format.
//     */
//    template <typename EA>
//    struct mkv_causal_graph : public ea::analysis::unary_function<EA> {
//        static const char* name() { return "mkv_causal_graph"; }
//        
//        virtual void operator()(EA& ea) {
//            using namespace ea;
//            using namespace ea::analysis;
//            typename EA::individual_type ind = analysis::find_most_fit_individual(ea);
//            markov_network net(get<MKV_INPUT_N>(ea), get<MKV_OUTPUT_N>(ea), get<MKV_HIDDEN_N>(ea), ea.rng());
//            build_markov_network(net, ind.repr().begin(), ind.repr().end(), ea);
//            datafile df(get<ANALYSIS_OUTPUT>(ea));
//            std::ostringstream title;
//            //            title << "individual=" << i.name() << "; generation=" << i.generation() << "; fitness=" << static_cast<std::string>(i.fitness());
//            mkv::write_graphviz(title.str(), df, mkv::as_causal_graph(net));
//        }
//    };
    
    /*! Datafile for markov network statistics.
     */
    template <typename EA>
    struct mkv_meta_population_datafile : ea::record_statistics_event<EA> {
        mkv_meta_population_datafile(EA& ea) : ea::record_statistics_event<EA>(ea), _df("mkv_meta_population_datafile.dat") {
            _df.add_field("update")
            .add_field("mean_gates")
            .add_field("max_gates")
            .add_field("mean_genome_size");
        }
        
        virtual ~mkv_meta_population_datafile() {
        }
        
        virtual void operator()(EA& ea) {
            using namespace boost::accumulators;
            using namespace ea;
            accumulator_set<double, stats<tag::mean,tag::max> > gates;
            accumulator_set<double, stats<tag::mean> > genes;
            
            for(typename EA::iterator i=ea.begin(); i!=ea.end(); ++i) {
                for(typename EA::individual_type::iterator j=i->begin(); j!=i->end(); ++j) {
                    mkv::markov_network net(get<MKV_INPUT_N>(ea), get<MKV_OUTPUT_N>(ea), get<MKV_HIDDEN_N>(ea), ea.rng());
                    mkv::build_markov_network(net, j->repr().begin(), j->repr().end(), ea);
                    
                    gates(net.size());
                    genes(j->repr().size());
                }
            }
            
            _df.write(ea.current_update())
            .write(mean(gates))
            .write(max(gates))
            .write(mean(genes))
            .endl();
        }
        
        ea::datafile _df;
    };
    
} // mkv

#endif

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
#ifndef _MKV_EA_MARKOV_NETWORK_H_
#define _MKV_EA_MARKOV_NETWORK_H_

#include <boost/algorithm/string/predicate.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/max.hpp>


#include <ea/meta_data.h>
#include <ea/analysis.h>
#include <ea/events.h>
#include <ea/datafile.h>

#include <mkv/markov_network.h>
#include <mkv/deep_markov_network.h>
#include <mkv/parse.h>
#include <mkv/graph.h>

// meta-data
LIBEA_MD_DECL(MKV_DESC, "markov_network.desc", std::string);
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
LIBEA_MD_DECL(GATE_HISTORY_FLOOR, "markov_network.gate.history.floor", int);
LIBEA_MD_DECL(GATE_HISTORY_LIMIT, "markov_network.gate.history.limit", int);
LIBEA_MD_DECL(GATE_WV_STEPS, "markov_network.gate.wv_steps", int);


namespace mkv {
    
    enum gate_types { MARKOV=42, LOGIC=43, ADAPTIVE=44, SPATIAL=45 };
    
    /*! Returns a set of supported gate types.
     */
    template <typename EA>
    std::set<gate_types> supported_gates(EA& ea) {
        using namespace ealib;
        std::set<gate_types> supported;
        std::string gates = get<MKV_GATE_TYPES>(ea);
        if(boost::icontains(gates,"markov")) { supported.insert(MARKOV); }
        if(boost::icontains(gates,"logic")) { supported.insert(LOGIC); }
        if(boost::icontains(gates,"adaptive")) { supported.insert(ADAPTIVE); }
        if(boost::icontains(gates,"spatial")) { supported.insert(SPATIAL); }
        return supported;
    }
}

namespace ealib {
	
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
    
    /*! Markov network mutation type.
     
     Performs per-site, duplication, and deletion mutations.
	 */
	struct mkv_smart_mutation {
		template <typename Representation, typename EA>
		void operator()(Representation& repr, EA& ea) {
            double per_site_p = get<MUTATION_PER_SITE_P>(ea);
            int imax = get<MUTATION_UNIFORM_INT_MAX>(ea);
            
            std::size_t f=1,l=repr.size(),last=0;
            if(f >= l) { return; }
            
            std::vector<std::pair<std::size_t, std::size_t> > genes;
            
            for( ; f!=l; ++f, ++last) {
                int start_codon = repr[f] + repr[last];
                if(start_codon == 255) {
                    std::size_t ms=ea.rng().uniform_integer(16, 257);
                    
                    // don't overrun:
                    if((f+ms) < l) {
                        // per-site mutations
                        for(std::size_t i=(f+1); i<(f+ms); ++i) {
                            if(ea.rng().p(per_site_p)) {
                                repr[i] = ea.rng()(imax);
                            }
                        }
                        
                        genes.push_back(std::make_pair(f-1, f+ms));
                    }
                }
            }
            
            if(!genes.empty()) {
                // gene duplication
                if(ea.rng().p(get<MUTATION_DUPLICATION_P>(ea)) && (repr.size()<get<MKV_REPR_MAX_SIZE>(ea))) {
                    std::pair<std::size_t, std::size_t>& g=*ea.rng().choice(genes.begin(), genes.end());
                    std::vector<typename Representation::codon_type> buffer(repr.begin()+g.first, repr.begin()+g.second);
                    repr.insert(repr.end(), buffer.begin(), buffer.end());
                }
                
                // gene deletion
                if(ea.rng().p(get<MUTATION_DELETION_P>(ea)) && (repr.size()>get<MKV_REPR_MIN_SIZE>(ea))) {
                    std::pair<std::size_t, std::size_t>& g=*ea.rng().choice(genes.begin(), genes.end());
                    repr.erase(repr.begin()+g.first, repr.begin()+g.second);
                }
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
        
        /*! Parse the inputs and outputs for a Markov network gate.
         */
        template <typename Network, typename ForwardIterator, typename MetaData>
        void build_io(Network& net, std::size_t& layer, index_list_type& inputs, index_list_type& outputs, ForwardIterator& h, MetaData& md) {
            using namespace ealib;
            using namespace ealib::algorithm;
            
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
        
        /*! Build a logic gate.
         */
        template <typename Network, typename ForwardIterator, typename MetaData>
        void build_logic_gate(Network& net, ForwardIterator h, MetaData& md) {
            std::size_t layer=0;
            index_list_type inputs, outputs;
            build_io(net, layer, inputs, outputs, h, md);
            logic_gate g(inputs, outputs, h);
            net.push_back(g);
        }
        
        /*! Build a markov gate.
         */
        template <typename Network, typename ForwardIterator, typename MetaData>
        void build_markov_gate(Network& net, ForwardIterator h, MetaData& md) {
            std::size_t layer=0;
            index_list_type inputs, outputs;
            build_io(net, layer, inputs, outputs, h, md);
            markov_gate g(inputs, outputs, h);
            net.push_back(g);
        }
        
        /*! Build an adaptive gate.
         */
        template <typename Network, typename ForwardIterator, typename MetaData>
        void build_adaptive_gate(Network& net, ForwardIterator h, MetaData& md) {
            using namespace ealib;
            using namespace ealib::algorithm;
            
            std::size_t layer=0;
            index_list_type inputs, outputs;
            build_io(net, layer, inputs, outputs, h, md);
            
            int nhistory=modnorm(*h++, get<GATE_HISTORY_FLOOR>(md), get<GATE_HISTORY_LIMIT>(md));
            int posf=*h++ % net.nstates();
            int negf=*h++ % net.nstates();
            
            weight_vector_type poswv(h, h+nhistory);
            std::transform(poswv.begin(), poswv.end(), poswv.begin(),
                           std::bind2nd(std::modulus<int>(), get<GATE_WV_STEPS>(md)+1));
            std::transform(poswv.begin(), poswv.end(), poswv.begin(),
                           std::bind2nd(std::multiplies<double>(), 1.0/get<GATE_WV_STEPS>(md)));
            h+=nhistory;
            weight_vector_type negwv(h, h+nhistory);
            std::transform(negwv.begin(), negwv.end(), negwv.begin(),
                           std::bind2nd(std::modulus<int>(), get<GATE_WV_STEPS>(md)+1));
            std::transform(negwv.begin(), negwv.end(), negwv.begin(),
                           std::bind2nd(std::multiplies<double>(), -1.0/get<GATE_WV_STEPS>(md)));
            h+=nhistory;
            
            adaptive_gate g(nhistory, posf, poswv, negf, negwv, inputs, outputs, h);
            net.push_back(g);
        }
        
        /*! Build a spatial gate.
         */
        template <typename Network, typename ForwardIterator, typename MetaData>
        void build_spatial_gate(Network& net, ForwardIterator h, MetaData& md) {
            std::size_t layer=0;
            index_list_type inputs, outputs;
            build_io(net, layer, inputs, outputs, h, md);
            
            for(std::size_t i=1; i<inputs.size();++i) {
                inputs[i] = inputs[i-1] + 1;
            }
            std::transform(inputs.begin(), inputs.end(), inputs.begin(), std::bind2nd(std::modulus<int>(), net.nstates()));
            
            logic_gate g(inputs, outputs, h);
            net.push_back(g);
        }
        
        template <typename ForwardIterator>
        std::size_t get_layer(ForwardIterator h, int max_layer) {
            using namespace ealib;
            using namespace ealib::algorithm;
            return modnorm(*h, 0, max_layer);
        }
        
    } // detail
    
    
    /*! Build a Deep Markov network from the genome [f,l), with the given meta data.
     */
    template <typename ForwardIterator, typename MetaData>
    void build_deep_markov_network(deep_markov_network& net, ForwardIterator f, ForwardIterator l, MetaData& md) {
        using namespace detail;
        using namespace ealib;
        using namespace ealib::algorithm;
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
                        markov_network& layer=net[get_layer(f+1,net.size())];
                        build_markov_gate(layer, f+2, md);
                        break;
                    }
                    case LOGIC: { // build a logic gate
                        if(!supported.count(LOGIC)) { break; }
                        markov_network& layer=net[get_layer(f+1,net.size())];
                        build_logic_gate(layer, f+2, md);
                        break;
                    }
                    case ADAPTIVE: { // build an adaptive gate
                        if(!supported.count(ADAPTIVE)) { break; }
                        markov_network& layer=net[get_layer(f+1,net.size())];
                        build_adaptive_gate(layer, f+2, md);
                        break;
                    }
                    case SPATIAL: { // build a spatial gate
                        if(!supported.count(SPATIAL)) { break; }
                        markov_network& layer=net[get_layer(f+1,net.size())];
                        build_spatial_gate(layer, f+2, md);
                        break;
                    }
                        
                    default: {
                        break;
                    }
                }
            }
        }
    }
    
    
    /*! Convenience method to build a Deep Markov Network.
     */
    template <typename ForwardIterator, typename RNG, typename EA>
    deep_markov_network make_deep_markov_network(const deep_markov_network::desc_type& desc,
                                                 ForwardIterator f, ForwardIterator l, RNG& rng, EA& ea) {
        deep_markov_network net(desc, rng);
        build_deep_markov_network(net, f, l, ea);
        return net;
    }
    
    
    /*! Build a Markov network from the genome [f,l), with the given meta data.
     */
    template <typename ForwardIterator, typename MetaData>
    void build_markov_network(markov_network& net, ForwardIterator f, ForwardIterator l, MetaData& md) {
        using namespace detail;
        using namespace ealib;
        using namespace ealib::algorithm;
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
                    case ADAPTIVE: { // build an adaptive gate
                        if(!supported.count(ADAPTIVE)) { break; }
                        build_adaptive_gate(net, f+1, md);
                        break;
                    }
                    case SPATIAL: { // build a spatial gate
                        if(!supported.count(SPATIAL)) { break; }
                        build_spatial_gate(net, f+1, md);
                        break;
                    }
                    default: {
                        break;
                    }
                }
            }
        }
    }
    
    /*! Convenience method to build a Markov Network.
     */
    template <typename ForwardIterator, typename RNG, typename EA>
    markov_network make_markov_network(const markov_network::desc_type& desc,
                                       ForwardIterator f, ForwardIterator l, RNG& rng, EA& ea) {
        markov_network net(desc, rng);
        build_markov_network(net, f, l, ea);
        return net;
    }
    
    /*! Convenience method to build a Markov Network.
     */
    template <typename ForwardIterator, typename RNG, typename EA>
    markov_network make_markov_network(ForwardIterator f, ForwardIterator l, RNG& rng, EA& ea) {
        markov_network::desc_type desc;
        parse_desc(get<MKV_DESC>(ea), desc);
        markov_network net(desc, rng);
        build_markov_network(net, f, l, ea);
        return net;
    }
    
    //    /*! Adapts the given mkv tool to a meta_population.
    //     */
    //    template <typename EA>
    //    struct mkv_meta_population_reduced_graph : public ealib::analysis::unary_function<EA> {
    //        static const char* name() { return "mkv_meta_population_reduced_graph"; }
    //
    //        virtual void operator()(EA& ea) {
    //            using namespace ealib;
    //            using namespace ealib::analysis;
    //
    //            int count=0;
    //            for(typename EA::iterator i=ea.begin(); i!=ea.end(); ++i) {
    //                typename EA::individual_type::individual_type& ind = analysis::find_dominant(*i);
    //                mkv::markov_network net = mkv::make_markov_network(ind.repr().begin(), ind.repr().end(), ea.rng(), ea);
    //
    //                datafile df("reduced_sp" + boost::lexical_cast<std::string>(count++) + ".dot");
    //                std::ostringstream title;
    //                title << "individual=" << ind.name() << ", generation=" << ind.generation() << ", fitness=" << ealib::fitness(ind,ea);
    //
    //                mkv::write_graphviz(title.str(), df, mkv::as_reduced_graph(net));
    //            }
    //        }
    //    };
    
    /*! Save the detailed graph of the dominant individual in graphviz format.
     */
    template <typename EA>
    struct mkv_genetic_graph : public ealib::analysis::unary_function<EA> {
        static const char* name() { return "mkv_genetic_graph";}
        
        virtual void operator()(EA& ea) {
            using namespace ealib;
            using namespace ealib::analysis;
            typename EA::individual_type& ind = analysis::find_dominant(ea);
            mkv::markov_network net = mkv::make_markov_network(ind.repr().begin(), ind.repr().end(), ea.rng(), ea);
            
            datafile df(get<ANALYSIS_OUTPUT>(ea,"mkv_genetic_graph.dot"));
            std::ostringstream title;
            title << "individual=" << ind.name() << ", generation=" << ind.generation() << ", fitness=" << ealib::fitness(ind,ea);
            mkv::write_graphviz(title.str(), df, mkv::as_genetic_graph(net));
        }
    };
            
    /*! Save the dominant individual in graphviz format.
     */
    template <typename EA>
    struct mkv_reduced_graph : public ealib::analysis::unary_function<EA> {
        static const char* name() { return "mkv_reduced_graph"; }
        
        virtual void operator()(EA& ea) {
            using namespace ealib;
            using namespace ealib::analysis;
            typename EA::individual_type& ind = analysis::find_dominant(ea);
            mkv::markov_network net = mkv::make_markov_network(ind.repr().begin(), ind.repr().end(), ea.rng(), ea);
            
            datafile df(get<ANALYSIS_OUTPUT>(ea,"mkv_reduced_graph.dot"));
            std::ostringstream title;
            title << "individual=" << ind.name() << ", generation=" << ind.generation() << ", fitness=" << ealib::fitness(ind,ea);
            mkv::write_graphviz(title.str(), df, mkv::as_reduced_graph(net));
        }
    };
    
    /*! Save the causal graph of the dominant individual in graphviz format.
     */
    template <typename EA>
    struct mkv_causal_graph : public ealib::analysis::unary_function<EA> {
        static const char* name() { return "mkv_causal_graph"; }
        
        virtual void operator()(EA& ea) {
            using namespace ealib;
            using namespace ealib::analysis;
            typename EA::individual_type& ind = analysis::find_dominant(ea);
            mkv::markov_network net = mkv::make_markov_network(ind.repr().begin(), ind.repr().end(), ea.rng(), ea);
            
            datafile df("mkv_causal_graph.dot");
            std::ostringstream title;
            title << "individual=" << ind.name() << ", generation=" << ind.generation() << ", fitness=" << ealib::fitness(ind,ea);
            mkv::write_graphviz(title.str(), df, mkv::as_causal_graph(net));
        }
    };
            
            /*! Datafile for markov network statistics.
             */
            //    template <typename EA>
            //    struct mkv_meta_population_datafile : ealib::record_statistics_event<EA> {
            //        mkv_meta_population_datafile(EA& ea) : ealib::record_statistics_event<EA>(ea), _df("mkv_meta_population_datafile.dat") {
            //            _df.add_field("update")
            //            .add_field("mean_gates")
            //            .add_field("max_gates")
            //            .add_field("mean_genome_size");
            //        }
            //
            //        virtual ~mkv_meta_population_datafile() {
            //        }
            //
            //        virtual void operator()(EA& ea) {
            //            using namespace boost::accumulators;
            //            using namespace ealib;
            //            accumulator_set<double, stats<tag::mean,tag::max> > gates;
            //            accumulator_set<double, stats<tag::mean> > genes;
            //
            //            for(typename EA::iterator i=ea.begin(); i!=ea.end(); ++i) {
            //                for(typename EA::individual_type::iterator j=i->begin(); j!=i->end(); ++j) {
            //                    mkv::markov_network net(get<MKV_INPUT_N>(ea), get<MKV_OUTPUT_N>(ea), get<MKV_HIDDEN_N>(ea), ea.rng());
            //                    mkv::build_markov_network(net, j->repr().begin(), j->repr().end(), ea);
            //
            //                    gates(net.ngates());
            //                    genes(j->repr().size());
            //                }
            //            }
            //
            //            _df.write(ea.current_update())
            //            .write(mean(gates))
            //            .write(max(gates))
            //            .write(mean(genes))
            //            .endl();
            //        }
            //
            //        ealib::datafile _df;
            //    };
            
            } // mkv
            
#endif

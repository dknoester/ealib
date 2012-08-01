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
#ifndef _FN_MARKOV_NETWORK_EA_H_
#define _FN_MARKOV_NETWORK_EA_H_

#include <ea/interface.h>
#include <ea/meta_data.h>
#include <ea/analysis.h>
#include <mkv/markov_network.h>
#include <mkv/detail/det_node.h>
#include <mkv/detail/prob_node.h>
#include <mkv/detail/adaptive_prob_node.h>

// meta-data
LIBEA_MD_DECL(MKV_INPUT_N, "markov_network.input.n", int);
LIBEA_MD_DECL(MKV_OUTPUT_N, "markov_network.output.n", int);
LIBEA_MD_DECL(MKV_HIDDEN_N, "markov_network.hidden.n", int);
LIBEA_MD_DECL(MKV_HOLD_TIME, "markov_network.hold_time", int);
LIBEA_MD_DECL(MKV_NODE_TYPES, "markov_network.node_types", std::string);
LIBEA_MD_DECL(MKV_INITIAL_NODES, "markov_network.initial_nodes", int);
LIBEA_MD_DECL(NODE_WV_STEPS, "markov_network.node.wv_steps", double);
LIBEA_MD_DECL(NODE_ALLOW_ZERO, "markov_network.node.allow_zero", bool);
LIBEA_MD_DECL(NODE_INPUT_LIMIT, "markov_network.node.input.limit", int);
LIBEA_MD_DECL(NODE_INPUT_FLOOR, "markov_network.node.input.floor", int);
LIBEA_MD_DECL(NODE_OUTPUT_LIMIT, "markov_network.node.output.limit", int);
LIBEA_MD_DECL(NODE_OUTPUT_FLOOR, "markov_network.node.output.floor", int);
LIBEA_MD_DECL(NODE_HISTORY_LIMIT, "markov_network.node.history.limit", int);
LIBEA_MD_DECL(NODE_HISTORY_FLOOR, "markov_network.node.history.floor", int);


enum { PROB=42, DET=43, ADAPTIVE=44, PROB_HIST=45 };


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
			if(ea.rng().p(get<MUTATION_DUPLICATION_P>(ea)) && (repr.size()<20000)) {
                int start = ea.rng().uniform_integer(0, repr.size());
                int extent = ea.rng().uniform_integer(16, 513);
                codon_buffer buf(extent);
                for(int i=0; i<extent; ++i) {
                    buf[i] = repr[start+i];
                }                
				repr.insert(ea.rng().choice(repr.begin(),repr.end()), buf.begin(), buf.end());
			}
            
            // gene deletion
			if(ea.rng().p(get<MUTATION_DELETION_P>(ea)) && (repr.size()>1000)) {
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
        typename EA::population_entry_type operator()(EA& ea) {
            typedef typename EA::representation_type representation_type;
            typename EA::individual_type ind;
            ind.name() = next<INDIVIDUAL_COUNT>(ea);
            ind.repr().resize(get<REPRESENTATION_SIZE>(ea), 127);            
            representation_type& repr=ind.repr();
			
            // which gate types are supported?
            std::set<unsigned int> supported;
            std::string gates = get<MKV_NODE_TYPES>(ea);
            if(gates.find("deterministic")!=std::string::npos) { supported.insert(DET); }
            if(gates.find("probabilistic")!=std::string::npos) { supported.insert(PROB); }
            if(gates.find("adaptive")!=std::string::npos) { supported.insert(ADAPTIVE); }
            if(gates.find("probhistorical")!=std::string::npos) { supported.insert(PROB_HIST); }
            
			int i,j;
			for(i=0; i<get<MKV_INITIAL_NODES>(ea); ++i) {
				j=ea.rng()(repr.size()-100);
                int gate=*ea.rng().choice(supported.begin(), supported.end()); //ea.rng()(3);
                repr[j] = gate;
                repr[j+1] = 255-gate;
				for(int k=2; k<20; ++k) {
					repr[j+k]=ea.rng()(256);
				}
			}
			return make_population_entry(ind,ea);
		}
	};
    
} //ea


namespace mkv {    

    template <typename Network, typename ForwardIterator, typename MetaData>
    void build_prob(Network& net, ForwardIterator h, MetaData& md) {
        using namespace detail;
        using namespace ea;
        using namespace ea::algorithm;

        int nin=modnorm(*h++, get<NODE_INPUT_FLOOR>(md), get<NODE_INPUT_LIMIT>(md));
        int nout=modnorm(*h++, get<NODE_OUTPUT_FLOOR>(md), get<NODE_OUTPUT_LIMIT>(md));
        index_list_type inputs(h, h+nin);
        std::transform(inputs.begin(), inputs.end(), inputs.begin(), std::bind2nd(std::modulus<int>(), net.svm_size()));
        h+=nin;
        index_list_type outputs(h, h+nout);
        std::transform(outputs.begin(), outputs.end(), outputs.begin(), std::bind2nd(std::modulus<int>(), net.svm_size()));
        h+=nout;
        
        markov_network::nodeptr_type p(new probabilistic_mkv_node(inputs, outputs, h, get<NODE_ALLOW_ZERO>(md)));
        net.append(p);
    }
    
    template <typename Network, typename ForwardIterator, typename MetaData>
    void build_det(Network& net, ForwardIterator h, MetaData& md) {
        using namespace detail;
        using namespace ea;
        using namespace ea::algorithm;
        
        int nin=modnorm(*h++, get<NODE_INPUT_FLOOR>(md), get<NODE_INPUT_LIMIT>(md));
        int nout=modnorm(*h++, get<NODE_OUTPUT_FLOOR>(md), get<NODE_OUTPUT_LIMIT>(md));
        index_list_type inputs(h, h+nin);
        std::transform(inputs.begin(), inputs.end(), inputs.begin(), std::bind2nd(std::modulus<int>(), net.svm_size()));
        h+=nin;
        index_list_type outputs(h, h+nout);
        std::transform(outputs.begin(), outputs.end(), outputs.begin(), std::bind2nd(std::modulus<int>(), net.svm_size()));
        h+=nout;
        
        markov_network::nodeptr_type p(new deterministic_mkv_node(inputs, outputs, h));
        net.append(p);
    }    
    
    template <typename Network, typename ForwardIterator, typename MetaData>
    void build_prob_hist(Network& net, ForwardIterator h, MetaData& md) {
        using namespace detail;
        using namespace ea;
        using namespace ea::algorithm;
        
        int nin=modnorm(*h++, get<NODE_INPUT_FLOOR>(md), get<NODE_INPUT_LIMIT>(md));
        int nout=modnorm(*h++, get<NODE_OUTPUT_FLOOR>(md), get<NODE_OUTPUT_LIMIT>(md));
        int nhistory=modnorm(*h++, get<NODE_HISTORY_FLOOR>(md), get<NODE_HISTORY_LIMIT>(md));
    
        index_list_type inputs(h, h+nin);
        std::transform(inputs.begin(), inputs.end(), inputs.begin(), std::bind2nd(std::modulus<int>(), net.svm_size()));
        h+=nin;
        index_list_type outputs(h, h+nout);
        std::transform(outputs.begin(), outputs.end(), outputs.begin(), std::bind2nd(std::modulus<int>(), net.svm_size()));
        h+=nout;
        
        markov_network::nodeptr_type p(new probabilistic_history_mkv_node(nhistory, inputs, outputs, h, get<NODE_ALLOW_ZERO>(md)));
        net.append(p);
    }
    
    template <typename Network, typename ForwardIterator, typename MetaData>
    void build_adaptive(Network& net, ForwardIterator h, MetaData& md) {
        using namespace detail;
        using namespace ea;
        using namespace ea::algorithm;
     
        int nin=modnorm(*h++, get<NODE_INPUT_FLOOR>(md), get<NODE_INPUT_LIMIT>(md));
        int nout=modnorm(*h++, get<NODE_OUTPUT_FLOOR>(md), get<NODE_OUTPUT_LIMIT>(md));
        int nhistory=modnorm(*h++, get<NODE_HISTORY_FLOOR>(md), get<NODE_HISTORY_LIMIT>(md));
        int posf=*h++ % net.svm_size();
        int negf=*h++ % net.svm_size();
        index_list_type inputs(h, h+nin);
        std::transform(inputs.begin(), inputs.end(), inputs.begin(), std::bind2nd(std::modulus<int>(), net.svm_size()));
        h+=nin;
        index_list_type outputs(h, h+nout);
        std::transform(outputs.begin(), outputs.end(), outputs.begin(), std::bind2nd(std::modulus<int>(), net.svm_size()));
        h+=nout;                        
        weight_vector_type poswv(h, h+nhistory);
        std::transform(poswv.begin(), poswv.end(), poswv.begin(), 
                       std::bind2nd(std::modulus<int>(), get<NODE_WV_STEPS>(md)+1));
        std::transform(poswv.begin(), poswv.end(), poswv.begin(), 
                       std::bind2nd(std::multiplies<double>(), 1.0/get<NODE_WV_STEPS>(md)));
        h+=nhistory;
        weight_vector_type negwv(h, h+nhistory);
        std::transform(negwv.begin(), negwv.end(), negwv.begin(), 
                       std::bind2nd(std::modulus<int>(), get<NODE_WV_STEPS>(md)+1));
        std::transform(negwv.begin(), negwv.end(), negwv.begin(), 
                       std::bind2nd(std::multiplies<double>(), -1.0/get<NODE_WV_STEPS>(md)));
        h+=nhistory;
        
        markov_network::nodeptr_type p(new synprob_mkv_node(nhistory,
                                                            posf, poswv,
                                                            negf, negwv,
                                                            inputs, outputs, h, get<NODE_ALLOW_ZERO>(md)));
        net.append(p);
    }
    

    /*! Build a Markov network from the genome [f,l), with the given meta data.
     */
    template <typename Network, typename ForwardIterator, typename MetaData>
    void build_markov_network(Network& net, ForwardIterator f, ForwardIterator l, MetaData& md) {
        using namespace detail;
        using namespace ea;
        using namespace ea::algorithm;
        
        if(f == l) { return; }
        
        // which gate types are supported?
        std::set<unsigned int> supported;
        std::string gates = get<MKV_NODE_TYPES>(md);
        if(gates.find("deterministic")!=std::string::npos) { supported.insert(DET); }
        if(gates.find("probabilistic")!=std::string::npos) { supported.insert(PROB); }
        if(gates.find("adaptive")!=std::string::npos) { supported.insert(ADAPTIVE); }
        if(gates.find("probhistorical")!=std::string::npos) { supported.insert(PROB_HIST); }
        
        ForwardIterator last=f;
        ++f;
        
        for( ; f!=l; ++f, ++last) {
            int start_codon = *f + *last;
            if(start_codon == 255) {
                switch(*last) {
                    case PROB: { // build a probabilistic node
                        if(!supported.count(PROB)) { break; }
                        build_prob(net, f+1, md);
                        break;
                    }
                    case DET: { // build a deterministic node
                        if(!supported.count(DET)) { break; }
                        build_det(net, f+1, md);
                        break;
                    }
                    case ADAPTIVE: { // build a synaptically learning probabilistic node
                        if(!supported.count(ADAPTIVE)) { break; }
                        build_adaptive(net, f+1, md);
                        break;
                    }
                    case PROB_HIST: {
                        if(!supported.count(PROB_HIST)) { break; }
                        build_prob_hist(net, f+1, md);
                        break;
                    }
                    default: { 
                        break;
                    }
                }
            }
        }
    }
    
    /*! Generates a random markov network.
     */
    template <typename Network, typename RNG, typename MetaData>
    void build_random_markov_network(Network& net, std::size_t n, RNG& rng, MetaData& md) {
        using namespace detail;
        using namespace ea;
        using namespace ea::algorithm;
        
        for( ; n>0; --n) {
            int nin=modnorm(rng(), get<NODE_INPUT_FLOOR>(md), get<NODE_INPUT_LIMIT>(md));
            int nout=modnorm(rng(), get<NODE_OUTPUT_FLOOR>(md), get<NODE_OUTPUT_LIMIT>(md));
            int hn=modnorm(rng(), get<NODE_HISTORY_FLOOR>(md), get<NODE_HISTORY_LIMIT>(md));
            
            index_list_type inputs(nin);
            std::generate(inputs.begin(), inputs.end(), rng);
            std::transform(inputs.begin(), inputs.end(), inputs.begin(), std::bind2nd(std::modulus<int>(), net.svm_size()));
            
            index_list_type outputs(nout);
            std::generate(outputs.begin(), outputs.end(), rng);
            std::transform(outputs.begin(), outputs.end(), outputs.begin(), std::bind2nd(std::modulus<int>(), net.svm_size()));
            
            std::vector<int> table((1<<nin) * (1<<nout));
            std::generate(table.begin(), table.end(), rng);
            
            markov_network::nodeptr_type p(new probabilistic_history_mkv_node(hn, inputs, outputs, table.begin(), get<NODE_ALLOW_ZERO>(md)));
            net.append(p);
        }
    }
    
} // mkv

#endif

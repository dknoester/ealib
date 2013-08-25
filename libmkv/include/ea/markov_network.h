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
#include <boost/lexical_cast.hpp>

#include <ea/configuration.h>
#include <ea/meta_data.h>
#include <ea/analysis.h>
#include <ea/events.h>
#include <ea/datafile.h>
#include <ea/representations/circular_genome.h>
#include <ea/mutation.h>

#include <mkv/markov_network.h>
#include <mkv/deep_markov_network.h>
#include <mkv/parse.h>
#include <mkv/graph.h>

namespace mkv {
    
    LIBEA_MD_DECL(MKV_DESC, "markov_network.desc", std::string);
    LIBEA_MD_DECL(MKV_UPDATE_N, "markov_network.update.n", int);
    LIBEA_MD_DECL(MKV_GATE_TYPES, "markov_network.gate_types", std::string);
    LIBEA_MD_DECL(MKV_GATE_ALLOW_PROBABILISTIC, "markov_network.gate.allow_probabilistic", int);
    LIBEA_MD_DECL(MKV_GATE_ALLOW_LOGIC, "markov_network.gate.allow_logic", int);
    LIBEA_MD_DECL(MKV_GATE_ALLOW_ADAPTIVE, "markov_network.gate.allow_adaptive", int);
    LIBEA_MD_DECL(MKV_INITIAL_GATES, "markov_network.initial_gates", int);
    LIBEA_MD_DECL(GATE_INPUT_LIMIT, "markov_network.gate.input.limit", int);
    LIBEA_MD_DECL(GATE_INPUT_FLOOR, "markov_network.gate.input.floor", int);
    LIBEA_MD_DECL(GATE_OUTPUT_LIMIT, "markov_network.gate.output.limit", int);
    LIBEA_MD_DECL(GATE_OUTPUT_FLOOR, "markov_network.gate.output.floor", int);
    LIBEA_MD_DECL(GATE_HISTORY_FLOOR, "markov_network.gate.history.floor", int);
    LIBEA_MD_DECL(GATE_HISTORY_LIMIT, "markov_network.gate.history.limit", int);
    LIBEA_MD_DECL(GATE_WV_STEPS, "markov_network.gate.wv_steps", int);
    
    enum gate_type { PROBABILISTIC=42, LOGIC=43, ADAPTIVE=44 };
    
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
        
        /*! Build a probabilistic gate.
         */
        template <typename Network, typename ForwardIterator, typename MetaData>
        void build_probabilistic_gate(Network& net, ForwardIterator h, MetaData& md) {
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
        
        template <typename ForwardIterator>
        std::size_t get_layer(ForwardIterator h, int max_layer) {
            using namespace ealib;
            using namespace ealib::algorithm;
            return modnorm(*h, 0, max_layer);
        }
        
        /*! Build a gate from f with the given meta data.
         */
        template <typename ForwardIterator, typename MetaData>
        void build_gate(int start_codon, markov_network& net, ForwardIterator f, MetaData& md) {
            switch(start_codon) {
                case PROBABILISTIC: { // build a markov gate
                    if(get<MKV_GATE_ALLOW_PROBABILISTIC>(md)) {
                        build_probabilistic_gate(net, f, md);
                    }
                    break;
                }
                case LOGIC: { // build a logic gate
                    if(get<MKV_GATE_ALLOW_LOGIC>(md)) {
                        build_logic_gate(net, f, md);
                    }
                    break;
                }
                case ADAPTIVE: { // build an adaptive gate
                    if(get<MKV_GATE_ALLOW_ADAPTIVE>(md)) {
                        build_adaptive_gate(net, f, md);
                    }
                    break;
                }
                default: {
                    break;
                }
            }
        }

    } // detail
    
    
    /*! Build a Deep Markov network from the genome [f,l), with the given meta data.
     */
    template <typename ForwardIterator, typename MetaData>
    void build_deep_markov_network(deep_markov_network& net, ForwardIterator f, ForwardIterator l, MetaData& md) {
        if(f == l) { return; }
        
        ForwardIterator last=f;
        ++f;
        
        for( ; f!=l; ++f, ++last) {
            int start_codon = *f + *last;
            if(start_codon == 255) {
                markov_network& layer=net[get_layer(f+1,net.size())];
                detail::build_gate(*last, layer, f+2, md);
            }
        }
    }
    
    
    /*! Build a Markov network from the genome [f,l), with the given meta data.
     */
    template <typename ForwardIterator, typename MetaData>
    void build_markov_network(markov_network& net, ForwardIterator f, ForwardIterator l, MetaData& md) {
        if(f == l) { return; }
        
        ForwardIterator last=f;
        ++f;
        
        for( ; f!=l; ++f, ++last) {
            int start_codon = *f + *last;
            if(start_codon == 255) {
                detail::build_gate(*last, net, f+1, md);
            }
        }
    }

    
    /*! Save the detailed graph of the dominant individual in graphviz format.
     */
    template <typename EA>
    struct genetic_graph : public ealib::analysis::unary_function<EA> {
        static const char* name() { return "genetic_graph";}
        
        virtual void operator()(EA& ea) {
            using namespace ealib;
            using namespace ealib::analysis;
            typename EA::individual_type& ind = analysis::find_dominant(ea);
            mkv::markov_network& net = ealib::phenotype(ind,ea.rng(),ea);
            
            datafile df(get<ANALYSIS_OUTPUT>(ea,"genetic_graph.dot"));
            mkv::write_graphviz(ind, df, mkv::as_genetic_graph(net));
        }
    };
    
    
    /*! Save the dominant individual in graphviz format.
     */
    template <typename EA>
    struct reduced_graph : public ealib::analysis::unary_function<EA> {
        static const char* name() { return "reduced_graph"; }
        
        virtual void operator()(EA& ea) {
            using namespace ealib;
            using namespace ealib::analysis;
            typename EA::individual_type& ind = analysis::find_dominant(ea);
            mkv::markov_network& net = ealib::phenotype(ind,ea.rng(),ea);
            
            datafile df(get<ANALYSIS_OUTPUT>(ea,"reduced_graph.dot"));
            mkv::write_graphviz(ind, df, mkv::as_reduced_graph(net));
        }
    };
    
    
    /*! Save the causal graph of the dominant individual in graphviz format.
     */
    template <typename EA>
    struct causal_graph : public ealib::analysis::unary_function<EA> {
        static const char* name() { return "causal_graph"; }
        
        virtual void operator()(EA& ea) {
            using namespace ealib;
            using namespace ealib::analysis;
            typename EA::individual_type& ind = analysis::find_dominant(ea);
            mkv::markov_network& net = ealib::phenotype(ind,ea.rng(),ea);
            
            datafile df("causal_graph.dot");
            mkv::write_graphviz(ind, df, mkv::as_causal_graph(net));
        }
    };
    
    
    /*! Save the dominant individual in graphviz format.
     */
    template <typename EA>
    struct deep_reduced_graph : public ealib::analysis::unary_function<EA> {
        static const char* name() { return "deep_reduced_graph"; }
        
        virtual void operator()(EA& ea) {
            using namespace ealib;
            using namespace ealib::analysis;
            typename EA::individual_type& ind = analysis::find_dominant(ea);
            mkv::deep_markov_network& net = ealib::phenotype(ind,ea.rng(),ea);

            for(std::size_t j=0; j<net.size(); ++j) {
                mkv::markov_network& layer=net[j];
                datafile df("reduced_l" + boost::lexical_cast<std::string>(j) + ".dot");
                mkv::write_graphviz(ind + boost::lexical_cast<std::string>(j), df, mkv::as_reduced_graph(layer));
            }
        }
    };
    
    
    /*! Datafile for markov networks statistics.
     */
    template <typename EA>
    struct network_statistics : ealib::analysis::unary_function<EA> {
        static const char* name() { return "network_statistics"; }
        
        virtual void operator()(EA& ea) {
            using namespace ealib;
            datafile df("network_statistics.dat");
            df.add_field("individual")
            .add_field("inputs")
            .add_field("outputs")
            .add_field("hidden")
            .add_field("gates");
            
            for(typename EA::iterator i=ea.begin(); i!=ea.end(); ++i) {
                mkv::markov_network& net = ealib::phenotype(*i,ea.rng(),ea);
                markov_graph G = as_reduced_graph(net);
                
                double gates=0.0, inputs=0.0, outputs=0.0, hidden=0.0;
                markov_graph::vertex_iterator vi,vi_end;
                for(boost::tie(vi,vi_end)=boost::vertices(G); vi!=vi_end; ++vi) {
                    if(!has_edges(*vi,G)) {
                        continue;
                    }
                    switch(G[*vi].nt) {
                        case vertex_properties::INPUT: {
                            ++inputs;
                            break;
                        }
                        case vertex_properties::OUTPUT: {
                            ++outputs;
                            break;
                        }
                        case vertex_properties::HIDDEN: {
                            ++hidden;
                            break;
                        }
                        case vertex_properties::GATE: {
                            ++gates;
                            break;
                        }
                        default: {
                            // shouldn't ever get here...
                            throw std::logic_error("markov_network.h::network_statistics: found a vertex with an invalid node_type.");
                        }
                    }
                }
                
                df.write(i->name()).write(inputs).write(outputs).write(hidden).write(gates).endl();
            }
        }
    };
    
    
    /*! Generates random Markov network-based individuals.
     */
    struct markov_network_ancestor {
        template <typename EA>
        typename EA::representation_type operator()(EA& ea) {
            using namespace ealib;
            
            typename EA::representation_type repr;
            repr.resize(get<REPRESENTATION_INITIAL_SIZE>(ea), 127);
            
            std::set<gate_type> supported;
            std::string gates = get<MKV_GATE_TYPES>(ea);
            if(boost::icontains(gates,"probabilistic")) { supported.insert(PROBABILISTIC); }
            if(boost::icontains(gates,"logic")) { supported.insert(LOGIC); }
            if(boost::icontains(gates,"adaptive")) { supported.insert(ADAPTIVE); }
            
            for(int i=0; i<get<MKV_INITIAL_GATES>(ea); ++i) {
                //                std::size_t csize=ea.rng()(get<MUTATION_INDEL_MIN_SIZE>(ea),
                //                                           get<MUTATION_INDEL_MAX_SIZE>(ea));
                std::size_t csize=get<MUTATION_INDEL_MAX_SIZE>(ea);

                int j=ea.rng()(repr.size()-csize);
                int gate=*ea.rng().choice(supported.begin(), supported.end());
                repr[j] = gate;
                repr[j+1] = 255-gate;
                for(std::size_t k=2; k<csize; ++k) {
                    repr[j+k]=ea.rng()(get<MUTATION_UNIFORM_INT_MIN>(ea), get<MUTATION_UNIFORM_INT_MAX>(ea));
                }
            }
            return repr;
        }
    };
    
    
    struct mkv_mutation {
        template <typename EA>
        void operator()(typename EA::individual_type& ind, EA& ea) {
            using namespace ealib;
            typename EA::representation_type& repr=ind.repr();
            typedef typename std::vector<typename EA::representation_type::codon_type> codon_buffer;
            
            double per_site_p = get<MUTATION_PER_SITE_P>(ea);
            int imax = get<MUTATION_UNIFORM_INT_MAX>(ea);
            for(typename EA::representation_type::iterator i=repr.begin(); i!=repr.end(); ++i) {
                if(ea.rng().p(per_site_p)) {
                    *i = ea.rng()(imax);
                }
            }
            
            // gene duplication
            // (the below looks a little crude, but there were some problems related
            // to incorrect compiler optimization.)
            if(ea.rng().p(get<MUTATION_INSERTION_P>(ea)) && (repr.size()<get<REPRESENTATION_MAX_SIZE>(ea))) {
                int start = ea.rng().uniform_integer(0, repr.size());
                int extent = ea.rng().uniform_integer(16, 513);
                codon_buffer buf(extent);
                for(int i=0; i<extent; ++i) {
                    buf[i] = repr[start+i];
                }
                repr.insert(ea.rng().choice(repr.begin(),repr.end()), buf.begin(), buf.end());
            }
            
            // gene deletion
            if(ea.rng().p(get<MUTATION_DELETION_P>(ea)) && (repr.size()>get<REPRESENTATION_MIN_SIZE>(ea))) {
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
            repr.resize(get<ealib::REPRESENTATION_INITIAL_SIZE>(ea), 127);
            
            // which gate types are supported?
            std::set<gate_type> supported;
            std::string gates = get<MKV_GATE_TYPES>(ea);
            if(boost::icontains(gates,"probabilistic")) { supported.insert(PROBABILISTIC); }
            if(boost::icontains(gates,"logic")) { supported.insert(LOGIC); }
            if(boost::icontains(gates,"adaptive")) { supported.insert(ADAPTIVE); }
            
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
    
    
    /*! Returns a set of supported gate types.
     */
    template <typename EA>
    void intialize_allowed_gates(EA& ea) {
        using namespace ealib;
        std::string gates = get<MKV_GATE_TYPES>(ea);
        put<MKV_GATE_ALLOW_PROBABILISTIC>(boost::icontains(gates,"probabilistic"), ea);
        put<MKV_GATE_ALLOW_LOGIC>(boost::icontains(gates,"logic"), ea);
        put<MKV_GATE_ALLOW_ADAPTIVE>(boost::icontains(gates,"adaptive"), ea);
    }
    
    /*! Configuration object for EAs that use Markov Networks.
     */
    template <typename EA>
    struct markov_network_configuration : public ealib::abstract_configuration<EA> {
        typedef ealib::indirectS encoding_type;
        typedef mkv::markov_network phenotype;
        typedef boost::shared_ptr<phenotype> phenotype_ptr;
        
        mkv::markov_network::desc_type mkv_desc;
        
        //! Translate an individual's representation into a Markov Network.
        virtual phenotype_ptr make_phenotype(typename EA::individual_type& ind,
                                             typename EA::rng_type& rng, EA& ea) {
            phenotype_ptr p(new mkv::markov_network(mkv_desc,rng));
            mkv::build_markov_network(*p, ind.repr().begin(), ind.repr().end(), ea);
            return p;
        }
        
        //! Called as the first step of an EA's lifecycle.
        virtual void configure(EA& ea) {
        }
        
        //! Called to generate the initial EA population.
        virtual void initial_population(EA& ea) {
            generate_ancestors(markov_network_ancestor(), get<ealib::POPULATION_SIZE>(ea), ea);
        }
        
        
        //! Called as the final step of EA initialization.
        virtual void initialize(EA& ea) {
            mkv::parse_desc(get<MKV_DESC>(ea), mkv_desc);
            intialize_allowed_gates(ea);
        }
    };
    
    
    /*! Configuration object for EAs that use Deep Markov Networks.
     */
    template <typename EA>
    struct deep_markov_network_configuration : public ealib::abstract_configuration<EA> {
        typedef ealib::indirectS encoding_type;
        typedef mkv::deep_markov_network phenotype;
        typedef boost::shared_ptr<phenotype> phenotype_ptr;
        
        mkv::deep_markov_network::desc_type mkv_desc;
        
        //! Translate an individual's representation into a Markov Network.
        virtual phenotype_ptr make_phenotype(typename EA::individual_type& ind,
                                             typename EA::rng_type& rng, EA& ea) {
            phenotype_ptr p(new mkv::deep_markov_network(mkv_desc,rng));
            mkv::build_deep_markov_network(*p, ind.repr().begin(), ind.repr().end(), ea);
            return p;
        }
        
        //! Called as the first step of an EA's lifecycle.
        virtual void configure(EA& ea) {
        }
        
        //! Called to generate the initial EA population.
        virtual void initial_population(EA& ea) {
            generate_ancestors(markov_network_ancestor(), get<ealib::POPULATION_SIZE>(ea), ea);
        }
        
        //! Called as the final step of EA initialization.
        virtual void initialize(EA& ea) {
            mkv::parse_desc(get<MKV_DESC>(ea), mkv_desc);
            intialize_allowed_gates(ea);
        }
    };
    
    typedef ealib::circular_genome<int> representation_type;
    typedef ealib::mutation::operators::indel<ealib::mutation::operators::per_site<ealib::mutation::site::uniform_integer> > mutation_type;
    //typedef mkv_mutation mutation_type;
    

} // mkv

#endif

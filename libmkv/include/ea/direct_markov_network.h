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
#ifndef _MKV_EA_DIRECT_MARKOV_NETWORK_H_
#define _MKV_EA_DIRECT_MARKOV_NETWORK_H_

#include <boost/algorithm/string/predicate.hpp>
#include <boost/variant.hpp>

#include <ea/configuration.h>
#include <ea/mutation.h>
#include <ea/cvector.h>

#include <ea/mkv/common.h>
#include <ea/mkv/analysis.h>
#include <ea/mkv/build.h>
#include <mkv/markov_network.h>
#include <mkv/parse.h>

namespace mkv {
    namespace detail {
        
        /*! Visitor used to mutate gates.
         */
        template <typename EA>
        class markov_network_mutation_visitor : public boost::static_visitor< > {
        public:
            markov_network_mutation_visitor(markov_network& net, EA& ea)
            : _net(net), _ea(ea), _per_site(0.0) {
                _per_site = get<ealib::MUTATION_PER_SITE_P>(_ea);
            }
            
            template <typename ForwardIterator>
            void mutate(ForwardIterator f, ForwardIterator l, int min, int max) {
                for( ; f!=l; ++f) {
                    if(_ea.rng().p(_per_site)) {
                        *f = static_cast<std::size_t>(_ea.rng().uniform_integer(min,max));
                    }
                }
            }

            template <typename ForwardIterator>
            void mutate(ForwardIterator f, ForwardIterator l, double min, double max) {
                for( ; f!=l; ++f) {
                    if(_ea.rng().p(_per_site)) {
                        *f = _ea.rng().uniform_real(min,max);
                    }
                }
            }

            void operator()(logic_gate& g) {
                // mutate inputs, outputs.  must be in correct range (nstates)
                mutate(g.inputs.begin(), g.inputs.end(), 0, static_cast<int>(_net.nstates()));
                mutate(g.outputs.begin(), g.outputs.end(), 0, static_cast<int>(_net.nstates()));
                // mutate logic table (arbitrary)
                mutate(g.M.begin(), g.M.end(), 0, std::numeric_limits<int>::max());
            }
            
            void operator()(markov_gate& g) {
                // mutate inputs, outputs.  must be in correct range (nstates)
                mutate(g.inputs.begin(), g.inputs.end(), 0, static_cast<int>(_net.nstates()));
                mutate(g.outputs.begin(), g.outputs.end(), 0, static_cast<int>(_net.nstates()));
                // mutate prob table, be sure to normalize
                mutate(g.M.data().begin(), g.M.data().end(), 0.0, 1.0);
                g.normalize();
            }
            
            void operator()(adaptive_gate& g)  {
                // mutate inputs, outputs.  must be in correct range (nstates)
                mutate(g.inputs.begin(), g.inputs.end(), 0, static_cast<int>(_net.nstates()));
                mutate(g.outputs.begin(), g.outputs.end(), 0, static_cast<int>(_net.nstates()));
                // mutate states controlling pos and neg feedback
                if(_ea.rng().p(_per_site)) {
                    g.p = static_cast<std::size_t>(_ea.rng().uniform_integer(0,static_cast<int>(_net.nstates())));
                }
                if(_ea.rng().p(_per_site)) {
                    g.n = static_cast<std::size_t>(_ea.rng().uniform_integer(0,static_cast<int>(_net.nstates())));
                }
                // mutate weight vectors
                mutate(g.P.begin(), g.P.end(), 0.0, 1.0);
                mutate(g.N.begin(), g.N.end(), -1.0, 0.0);
                
                // mutate prob table, be sure to normalize
                mutate(g.M.data().begin(), g.M.data().end(), 0.0, 1.0);
                g.normalize();
            }
            
            markov_network& _net;
            EA& _ea;
            double _per_site;
        };
        
        struct direct_mutate_markov_network {
            template <typename EA>
            void operator()(typename EA::individual_type& ind, EA& ea) {
                using namespace ealib;
                markov_network& net=ind.repr();
                
                if((net.size() < get<MKV_MAX_GATES>(ea)) && ea.rng().p(get<MUTATION_INSERTION_P>(ea))) {
                    insert_gate(net,ea);
                }
                
                if((net.size() < get<MKV_MAX_GATES>(ea)) && ea.rng().p(get<MUTATION_DUPLICATION_P>(ea))) {
                    duplicate_gate(net,ea);
                }
                
                if((net.size() > get<MKV_MIN_GATES>(ea)) && ea.rng().p(get<MUTATION_DELETION_P>(ea))) {
                    delete_gate(net,ea);
                }
                
                markov_network_mutation_visitor<EA> visitor(net, ea);
                for(std::size_t i=0; i<net.size(); ++i) {
                    net[i].apply_visitor(visitor);
                }
            }
            
            template <typename EA>
            void insert_gate(markov_network& net, EA& ea) {
                ealib::cvector<int> r;
                std::generate_n(std::back_inserter(r), 1000, ea.rng()); // it's a circular list, so even if we need more, 'tis ok.
                switch(*ea.rng().choice(ea.configuration().supported_gates.begin(),
                                        ea.configuration().supported_gates.end())) {
                    case PROBABILISTIC: {
                        build_probabilistic_gate(net, r.begin(), ea);
                        break;
                    }
                    case LOGIC: {
                        build_logic_gate(net, r.begin(), ea);
                        break;
                    }
                    case ADAPTIVE: {
                        build_adaptive_gate(net, r.begin(), ea);
                        break;
                    }
                    default:
                        throw ealib::bad_argument_exception("direct_markov_network.h: unknown gate type");
                }
            }
            
            template <typename EA>
            void duplicate_gate(markov_network& net, EA& ea) {
                net.push_back(*ea.rng().choice(net.begin(), net.end()));
            }
            
            template <typename EA>
            void delete_gate(markov_network& net, EA& ea) {
                net.erase(ea.rng().choice(net.begin(), net.end()));
            }
        };
        
        /*! Generates random Markov network-based individuals.
         */
        struct direct_markov_network_ancestor {
            template <typename EA>
            typename EA::representation_type operator()(EA& ea) {
                using namespace ealib;
                markov_network net(ea.configuration().mkv_desc, ea.rng().seed());
                
                direct_mutate_markov_network mt;
                for(std::size_t i=0; i<get<MKV_INITIAL_GATES>(ea); ++i) {
                    mt.insert_gate(net,ea);
                }
                return net;
            }
        };
        
    } // detail

    
    /*! Configuration object for EAs that use Markov Networks.
     */
    template <typename EA>
    struct markov_network_configuration : public ealib::abstract_configuration<EA> {
        typedef ealib::directS encoding_type;
        typedef typename EA::representation_type phenotype;
        typedef phenotype* phenotype_ptr;
        
        mkv::markov_network::desc_type mkv_desc;
        std::set<gate_type> supported_gates;
        
        //! Called as the first step of an EA's lifecycle.
        virtual void configure(EA& ea) {
        }
        
        //! Called to generate the initial EA population.
        virtual void initial_population(EA& ea) {
            generate_ancestors(detail::direct_markov_network_ancestor(), get<ealib::POPULATION_SIZE>(ea), ea);
        }
        
        
        //! Called as the final step of EA initialization.
        virtual void initialize(EA& ea) {
            mkv::parse_desc(get<MKV_DESC>(ea), mkv_desc);
            
            std::string gates = get<MKV_GATE_TYPES>(ea);
            if(boost::icontains(gates,"probabilistic")) {
                supported_gates.insert(PROBABILISTIC);
            }
            if(boost::icontains(gates,"logic")) {
                supported_gates.insert(LOGIC);
            }
            if(boost::icontains(gates,"adaptive")) {
                supported_gates.insert(ADAPTIVE);
            }
        }
    };
    
    typedef markov_network representation_type;
    typedef detail::direct_mutate_markov_network mutation_type;
    
    /*! Add the common Markov Network configuration options to the command line interface.
     */
    template <typename EA>
    void add_options(ealib::cmdline_interface<EA>* ci) {
        using namespace ealib;
        // markov network options
        add_option<MKV_DESC>(ci);
        add_option<MKV_UPDATE_N>(ci);
        add_option<MKV_GATE_TYPES>(ci);
        add_option<MKV_INITIAL_GATES>(ci);
        add_option<MKV_MAX_GATES>(ci);
        add_option<MKV_MIN_GATES>(ci);
        add_option<GATE_INPUT_LIMIT>(ci);
        add_option<GATE_INPUT_FLOOR>(ci);
        add_option<GATE_OUTPUT_LIMIT>(ci);
        add_option<GATE_OUTPUT_FLOOR>(ci);
        add_option<GATE_HISTORY_LIMIT>(ci);
        add_option<GATE_HISTORY_FLOOR>(ci);
        add_option<GATE_WV_STEPS>(ci);
        
        // ea options
        add_option<MUTATION_PER_SITE_P>(ci);
        add_option<MUTATION_DELETION_P>(ci);
        add_option<MUTATION_INSERTION_P>(ci);
        add_option<MUTATION_DUPLICATION_P>(ci);
    }

} // mkv

#endif

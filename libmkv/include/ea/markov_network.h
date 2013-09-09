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

#include <ea/cmdline_interface.h>
#include <ea/configuration.h>
#include <ea/mutation.h>
#include <ea/representations/circular_genome.h>

#include <ea/mkv/common.h>
#include <ea/mkv/analysis.h>
#include <ea/mkv/build.h>
#include <mkv/markov_network.h>
#include <mkv/parse.h>

namespace mkv {
    
    /*! Generates random Markov network-based individuals.
     */
    struct markov_network_ancestor {
        template <typename EA>
        typename EA::representation_type operator()(EA& ea) {
            using namespace ealib;
            
            typename EA::representation_type repr;
            repr.resize(get<REPRESENTATION_INITIAL_SIZE>(ea), 127);
            
            for(std::size_t i=0; i<get<MKV_INITIAL_GATES>(ea); ++i) {
                std::size_t csize=ea.rng()(get<MUTATION_INDEL_MIN_SIZE>(ea),
                                           get<MUTATION_INDEL_MAX_SIZE>(ea));
                int j=ea.rng()(repr.size()-csize);
                int gate=*ea.rng().choice(ea.configuration().supported_gates.begin(), ea.configuration().supported_gates.end());
                repr[j] = gate;
                repr[j+1] = 255-gate;
                for(std::size_t k=2; k<csize; ++k) {
                    repr[j+k]=ea.rng()(get<MUTATION_UNIFORM_INT_MIN>(ea), get<MUTATION_UNIFORM_INT_MAX>(ea));
                }
            }
            return repr;
        }
    };
    
    
    /*! Configuration object for EAs that use Markov Networks.
     */
    template <typename EA>
    struct markov_network_configuration : public ealib::abstract_configuration<EA> {
        typedef ealib::indirectS encoding_type;
        typedef mkv::markov_network phenotype;
        typedef boost::shared_ptr<phenotype> phenotype_ptr;
        
        mkv::markov_network::desc_type mkv_desc;
        std::set<gate_type> supported_gates;
        
        //! Translate an individual's representation into a Markov Network.
        virtual phenotype_ptr make_phenotype(typename EA::individual_type& ind,
                                             typename EA::rng_type& rng, EA& ea) {
            phenotype_ptr p(new mkv::markov_network(mkv_desc,rng));
            detail::build_markov_network(*p, ind.repr().begin(), ind.repr().end(), ea, supported_gates);
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
    
    typedef ealib::circular_genome<int> representation_type;
    typedef ealib::mutation::operators::indel<ealib::mutation::operators::per_site<ealib::mutation::site::uniform_integer> > mutation_type;
    
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
        add_option<GATE_INPUT_LIMIT>(ci);
        add_option<GATE_INPUT_FLOOR>(ci);
        add_option<GATE_OUTPUT_LIMIT>(ci);
        add_option<GATE_OUTPUT_FLOOR>(ci);
        add_option<GATE_HISTORY_LIMIT>(ci);
        add_option<GATE_HISTORY_FLOOR>(ci);
        add_option<GATE_WV_STEPS>(ci);
        
        // ea options
        add_option<REPRESENTATION_INITIAL_SIZE>(ci);
        add_option<REPRESENTATION_MIN_SIZE>(ci);
        add_option<REPRESENTATION_MAX_SIZE>(ci);
        add_option<MUTATION_PER_SITE_P>(ci);
        add_option<MUTATION_UNIFORM_INT_MIN>(ci);
        add_option<MUTATION_UNIFORM_INT_MAX>(ci);
        add_option<MUTATION_DELETION_P>(ci);
        add_option<MUTATION_INSERTION_P>(ci);
        add_option<MUTATION_INDEL_MIN_SIZE>(ci);
        add_option<MUTATION_INDEL_MAX_SIZE>(ci);
    }
    
} // mkv

#endif

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
#include <ea/mutation.h>
#include <ea/representations/circular_genome.h>

#include <mkv/markov_network.h>
#include <mkv/deep_markov_network.h>
#include <mkv/parse.h>
#include <ea/mkv/common.h>
#include <ea/mkv/analysis.h>
#include <ea/mkv/build.h>
#include <ea/markov_network.h>

namespace mkv {
    
    /*! Configuration object for EAs that use Deep Markov Networks.
     */
    template <typename EA>
    struct deep_markov_network_configuration : public ealib::abstract_configuration<EA> {
        typedef ealib::indirectS encoding_type;
        typedef mkv::deep_markov_network phenotype;
        typedef boost::shared_ptr<phenotype> phenotype_ptr;
        
        mkv::deep_markov_network::desc_type mkv_desc;
        gate_selector_type supported_gates;
        
        //! Translate an individual's representation into a Markov Network.
        virtual phenotype_ptr make_phenotype(typename EA::individual_type& ind,
                                             typename EA::rng_type& rng, EA& ea) {
            phenotype_ptr p(new mkv::deep_markov_network(mkv_desc,rng));
            detail::build_deep_markov_network(*p, ind.repr().begin(), ind.repr().end(), ea, supported_gates);
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
    
} // mkv

#endif

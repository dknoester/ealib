/* mkv/markov_evolution_algorithm.h
 *
 * This file is part of EALib.
 *
 * Copyright 2014 David B. Knoester.
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
#ifndef _MKV_EA_MARKOV_NETWORK_EVOLUTION_H_
#define _MKV_EA_MARKOV_NETWORK_EVOLUTION_H_

#include <boost/algorithm/string/predicate.hpp>
#include <ea/evolutionary_algorithm.h>

#include <ea/lifecycle.h>
#include <ea/cmdline_interface.h>
#include <ea/functional.h>
#include <ea/genome_types/circular_genome.h>
#include <ea/metadata.h>
#include <mkv/markov_network.h>
#include <mkv/translator.h>
#include <mkv/analysis.h>

namespace mkv {
    using namespace ealib;
	LIBEA_MD_DECL(MKV_UPDATE_N, "markov_network.update.n", std::size_t);
    LIBEA_MD_DECL(MKV_INPUT_N, "markov_network.input.n", std::size_t);
    LIBEA_MD_DECL(MKV_OUTPUT_N, "markov_network.output.n", std::size_t);
    LIBEA_MD_DECL(MKV_HIDDEN_N, "markov_network.hidden.n", std::size_t);
    LIBEA_MD_DECL(MKV_INITIAL_GATES, "markov_network.initial_gates", std::size_t);
    LIBEA_MD_DECL(MKV_GATE_TYPES, "markov_network.gate_types", std::string);


    //! Generates random Markov network-based individuals.
    struct markov_network_ancestor {
        template <typename EA>
        typename EA::genome_type operator()(EA& ea) {
            typename EA::genome_type repr;
            repr.resize(get<REPRESENTATION_INITIAL_SIZE>(ea), 127);
            
            for(std::size_t i=0; i<get<MKV_INITIAL_GATES>(ea); ++i) {
                std::size_t csize=ea.rng()(get<MUTATION_INDEL_MIN_SIZE>(ea),
                                           get<MUTATION_INDEL_MAX_SIZE>(ea));
                int j=ea.rng()(repr.size()-csize);
                markov_network_translator& t = ea.lifecycle().translator;
                int gate=*ea.rng().choice(t.enabled().begin(), t.enabled().end());
                repr[j] = gate;
                repr[j+1] = 255-gate;
                for(std::size_t k=2; k<csize; ++k) {
                    repr[j+k]=ea.rng()(get<MUTATION_UNIFORM_INT_MIN>(ea), get<MUTATION_UNIFORM_INT_MAX>(ea));
                }
            }
            return repr;
        }
    };
    
    //! Configuration object for EAs that use Markov Networks.
    struct markov_network_lifecycle : ealib::default_lifecycle {
        //! Called after EA initialization.
        template <typename EA>
        void initialize(EA& ea) {
            const std::string& gates = get<MKV_GATE_TYPES>(ea);
            if(!boost::algorithm::icontains(gates, "logic")) {
                translator.disable(LOGIC);
            }
            if(!boost::algorithm::icontains(gates, "probabilistic")) {
                translator.disable(PROBABILISTIC);
            }
            if(!boost::algorithm::icontains(gates, "adaptive")) {
                translator.disable(ADAPTIVE);
            }
        }
        
        markov_network_translator translator; //!< Genome translator.
    };
    
    //! Call the Markov network translator.
    struct call_markov_network_translator {
        template <typename EA>
        call_markov_network_translator(EA& ea) {
        }
        
        //! Translate the given genome into an L-System.
        template <typename Genome, typename Phenotype, typename EA>
        void operator()(Genome& G, Phenotype& P, EA& ea) {
            P.resize(get<MKV_INPUT_N>(ea),
                     get<MKV_OUTPUT_N>(ea),
                     get<MKV_HIDDEN_N>(ea));
            ea.lifecycle().translator.translate_genome(P,G);
        }
    };
    
    //! Add the common Markov Network configuration options to the command line interface.
    template <typename EA>
    void add_options(cmdline_interface<EA>* ci) {
        using namespace ealib;
        // markov network options
        add_option<MKV_UPDATE_N>(ci);
        add_option<MKV_INPUT_N>(ci);
        add_option<MKV_OUTPUT_N>(ci);
        add_option<MKV_HIDDEN_N>(ci);
        add_option<MKV_INITIAL_GATES>(ci);
        add_option<MKV_GATE_TYPES>(ci);
        
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

	/*! Markov network evolutionary algorithm.
     
     This class specializes evolutionary_algorithm to provide an algorithm specific
     to evolving Markov networks.  If more advanced control over the features of
     the GA are needed, the reader is referred to evolutionary_algorithm.h.
	 */
	template
    < typename FitnessFunction
	, typename RecombinationOperator
	, typename GenerationalModel
    , typename StopCondition=dont_stop
    , typename PopulationGenerator=fill_population
    , typename Lifecycle=markov_network_lifecycle
    , template <typename> class Traits=fitness_trait
    > class markov_network_evolution
    : public evolutionary_algorithm
    < indirect<circular_genome<int>, markov_network< >, call_markov_network_translator>
    , FitnessFunction
    , mutation::operators::indel<mutation::operators::per_site<mutation::site::uniform_integer> >
    , RecombinationOperator
    , GenerationalModel
    , markov_network_ancestor
    , StopCondition
    , PopulationGenerator
    , Lifecycle
    , Traits
    > {
    };

} // mkv

#endif

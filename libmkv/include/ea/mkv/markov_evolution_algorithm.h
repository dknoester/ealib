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
#ifndef _EA_MKV_MARKOV_EVOLUTION_ALGORITHM_H_
#define _EA_MKV_MARKOV_EVOLUTION_ALGORITHM_H_
#include <boost/algorithm/string/predicate.hpp>
#include <ea/lifecycle.h>
#include <ea/cmdline_interface.h>
#include <ea/functional.h>
#include <ea/genome_types/circular_genome.h>
#include <ea/mutation.h>
#include <ea/traits.h>
#include <ea/translation.h>

#include <ea/evolutionary_algorithm.h>
#include <ea/stopping.h>
#include <ea/ancestors.h>
#include <mkv/markov_network.h>

namespace ealib {
    namespace ealib {
        namespace mkv {
            enum { IN, OUT, HID }; //!< Indices into the desc_type for number of inputs, outputs, and hidden states.
            
            //! Descriptor (ninput, noutput, nhidden) for a Markov network.
            typedef boost::tuple<std::size_t, std::size_t, std::size_t> desc_type;
        } // mkv
        

    
    //    namespace mkv {
    //
    //        /*! Configuration object for EAs that use Markov Networks.
    //         */
    //        struct configuration : ealib::default_configuration {
    //            //! Called after EA initialization.
    //            template <typename EA>
    //            void initialize(EA& ea) {
    //                desc = desc_type(get<MKV_INPUT_N>(ea), get<MKV_OUTPUT_N>(ea), get<MKV_HIDDEN_N>(ea));
    //            }
    //
    //            //! Disable a gate type.
    //            void disable(gate_type g) {
    //                translator.disable(g);
    //            }
    //
    //            desc_type desc; //!< Description for Markov network (# in, out, & hidden).
    //            start_codon start; //!< Start codon detector.
    //            genome_translator translator; //!< Genome translator.
    //        };
    //
    //        /*! Markov network specific traits for an individual.
    //         */
    //        template <typename T>
    //        struct default_traits : ealib::default_traits<T> {
    //            //! Translate an individual's representation into a Markov Network.
    //            template <typename EA>
    //            typename EA::phenotype_ptr_type make_phenotype(typename EA::individual_type& ind, EA& ea) {
    //                typename EA::phenotype_ptr_type p(new typename EA::phenotype_type(ea.config().desc));
    //                translate_genome(ind.repr(), ea.config().start, ea.config().translator, *p);
    //                return p;
    //            }
    //        };
    //
    //        /*! Markov network specific traits for an individual.
    //         */
    //        template <typename T>
    //        struct lod_default_traits : ealib::default_lod_traits<T> {
    //            //! Translate an individual's representation into a Markov Network.
    //            template <typename EA>
    //            typename EA::phenotype_ptr_type make_phenotype(typename EA::individual_type& ind, EA& ea) {
    //                typename EA::phenotype_ptr_type p(new typename EA::phenotype_type(ea.config().desc));
    //                translate_genome(ind.repr(), ea.config().start, ea.config().translator, *p);
    //                return p;
    //            }
    //        };
    
    typedef circular_genome<int> representation_type;
    typedef mutation::operators::indel<mutation::operators::per_site<mutation::site::uniform_integer> > mutation_type;
    
    /*! Add the common Markov Network configuration options to the command line interface.
     */
    template <typename EA>
    void add_options(cmdline_interface<EA>* ci) {
        using namespace ealib;
        // markov network options
        add_option<MKV_LAYERS_N>(ci);
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
    
} // mkv
    
	/*! Markov network evolutionary algorithm.
     
     This class specializes evolutionary_algorithm to provide an algorithm specific
     to evolving Markov networks.  If more advanced control over the features of
     the GA are needed, the reader is referred to evolutionary_algorithm.h.
	 */
	template
    < typename FitnessFunction
	, typename RecombinationOperator
	, typename GenerationalModel
    , typename EarlyStopCondition=dont_stop
    , typename UserDefinedConfiguration=mkv::configuration
    , typename PopulationGenerator=fill_population
    > class markov_evolution_algorithm
    : public evolutionary_algorithm
    < individual<mkv::representation_type, FitnessFunction, markov_network< >, indirectS, mkv::default_traits>
    , mkv::ancestor_generator
    , mkv::mutation_type
    , RecombinationOperator
    , GenerationalModel
    , EarlyStopCondition
    , UserDefinedConfiguration
    , PopulationGenerator
    > {
    };

    /*! Markov network evolutionary algorithm.
     
     This class specializes evolutionary_algorithm to provide an algorithm specific
     to evolving Markov networks.  If more advanced control over the features of
     the GA are needed, the reader is referred to evolutionary_algorithm.h.
	 */
	template
    < typename FitnessFunction
	, typename RecombinationOperator
	, typename GenerationalModel
    , typename EarlyStopCondition=dont_stop
    , typename UserDefinedConfiguration=mkv::configuration
    , typename PopulationGenerator=fill_population
    > class markov_evolution_lod_algorithm
    : public evolutionary_algorithm
    < individual<mkv::representation_type, FitnessFunction, markov_network< >, indirectS, mkv::lod_default_traits>
    , mkv::ancestor_generator
    , mkv::mutation_type
    , RecombinationOperator
    , GenerationalModel
    , EarlyStopCondition
    , UserDefinedConfiguration
    , PopulationGenerator
    > {
    };
    
} // ealib

#endif

/* artificial_life/artificial_life.h 
 * 
 * This file is part of EALib.
 * 
 * Copyright 2012 David B. Knoester, Heather J. Goldsby.
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

#ifndef _EA_ARTIFICIAL_LIFE_ARTIFICIAL_LIFE_H_
#define _EA_ARTIFICIAL_LIFE_ARTIFICIAL_LIFE_H_

#include <boost/serialization/nvp.hpp>
#include <boost/shared_ptr.hpp>

#include <ea/concepts.h>
#include <ea/artificial_life/organism.h>
#include <ea/artificial_life/schedulers.h>
#include <ea/artificial_life/replication.h>
#include <ea/artificial_life/well_mixed.h>
#include <ea/artificial_life/task_library.h>
#include <ea/initialization.h>
#include <ea/interface.h>
#include <ea/meta_data.h>
#include <ea/mutation.h>
#include <ea/population.h>
#include <ea/recombination.h>
#include <ea/events.h>
#include <ea/rng.h>


namespace ea {
    
    /*! Generates the default ancestor.
     */
    struct repro_ancestor {
        template <typename EA>
        typename EA::population_entry_type operator()(EA& ea) {
            typedef typename EA::representation_type representation_type;
            typename EA::individual_type ind;
            ind.name() = next<INDIVIDUAL_COUNT>(ea);
            ind.repr().resize(get<REPRESENTATION_SIZE>(ea));
            representation_type& repr=ind.repr();
            
            std::fill(ind.repr().begin(), ind.repr().end(), 3);
            *ind.repr().rbegin() = 13;
            return make_population_entry(ind, ea);
        }
    };
    
    /*! Generates the nop-x ancestor.
     */
    struct nopx_ancestor {
        template <typename EA>
        typename EA::population_entry_type operator()(EA& ea) {
            typedef typename EA::representation_type representation_type;
            typename EA::individual_type ind;
            ind.name() = next<INDIVIDUAL_COUNT>(ea);
            
            representation_type& repr=ind.repr();
            repr.resize(get<REPRESENTATION_SIZE>(ea));
            std::fill(repr.begin(), repr.end(), 3);
            return make_population_entry(ind, ea);
        }
    };


    /*! Initialization method that generates a complete population.
     */
    template <typename IndividualGenerator>
    struct alife_population {
        template <typename EA>
        void operator()(EA& ea) {
            typename EA::population_type ancestral;
            typename EA::individual_type a = typename EA::individual_type();
            a.name() = next<INDIVIDUAL_COUNT>(ea);
            a.generation() = -1.0;
            a.update() = ea.current_update();
            ancestral.append(make_population_entry(a,ea));
            
            IndividualGenerator ig;
            ea.population().clear();
            generate_individuals_n(ea.population(), ig, get<INITIAL_POPULATION_SIZE>(ea), ea);
            
            for(typename EA::population_type::iterator i=ea.population().begin(); i!=ea.population().end(); ++i) {
                ea.events().inheritance(ancestral,ind(i,ea),ea);
                ea.env().insert(*i);
                ind(i,ea).priority() = 1.0;
            }
        }
    };
    

    template <typename EA>
	struct alife_event_handler : event_handler<EA> {
		/* life history events */

        //! 
        boost::signal<void(typename EA::individual_type&, // individual
                           double, // amount of resource consumed
                           const std::string&, // task name
                           EA&)> task_performed;

        //! Called when an individual is "born" (immediately after it is placed in the population).
        boost::signal<void(typename EA::individual_type&, // individual
                           EA&)> birth;

        //! Called when an individual "dies" or is replaced.
        boost::signal<void(typename EA::individual_type&, // individual
                           EA&)> death;
    };

    
    /*! Artificial life top-level evolutionary algorithm.
     
     The key difference between artificial life and standard evolutionary algorithms
     is that individuals here are scheduled for execution, as opposed to having
     their fitness evaluated.  This means that each individual is "visited" more
     than once during each generation, and in fact, the traditional (EA) fitness
     is a function of both the individual's behavior and the population in which
     it lives.  Moreover, replication in an artificial life system is driven 
     by the individual, instead of externally (e.g., by a generational model).
     
     A final complicating factor is that individuals in artificial life interact
     through an "environment."  Such environments are typically responsible for
     handling topology, resource gradients, etc.
     
     While these differences *could* be incorporated into a traditional evolutionary
     algorithm, this could not be done without bastardizing many of the concepts
     that are typically found in an EA.  Thus, the artificial_life class found
     here.
     
     In general, the design of this class is based on concepts from the Avida 
     platform for digital evolution~\cite{ofria2004}.
     
     In order to preserve as much compatibility between EA and AL components,
     the "organisms" in AL are referred to as "individuals."
     */
	template <
    typename Hardware,
    template <typename> class InstructionSet,
    template <typename> class Environment=well_mixed,
    typename ReplacementStrategy=first_neighbor,
    template <typename> class Scheduler=weighted_round_robin,
	typename MutationOperator=mutation::per_site<mutation::uniform_integer>,
    template <typename> class TaskLibrary=task_library,
    template <typename> class Individual=organism,
	template <typename, typename> class Population=population,
	typename Initializer=alife_population<repro_ancestor>,
	template <typename> class EventHandler=alife_event_handler,
	typename MetaData=meta_data,
	typename RandomNumberGenerator=ea::default_rng_type>
    class artificial_life {
    public:
        //! Hardware type.
        typedef Hardware hardware_type;
        //! Representation type.
        typedef typename hardware_type::representation_type representation_type;
        //! Scheduler type.
        typedef Scheduler<artificial_life> scheduler_type;
        //! Scheduler fitness type.
        typedef typename scheduler_type::priority_type priority_type;
        //! Individual type.
        typedef Individual<artificial_life> individual_type;
        //! Individual pointer type.
        typedef boost::shared_ptr<individual_type> individual_ptr_type;
        //! ISA type.
        typedef InstructionSet<artificial_life> isa_type;
        //! Replacment strategy type.
        typedef ReplacementStrategy replacement_type;
        //! Environment type.
        typedef Environment<artificial_life> environment_type;
        //! Task library type.
        typedef TaskLibrary<artificial_life> tasklib_type;
        //! Mutation operator type.
        typedef MutationOperator mutation_operator_type;
        //! Population type.
        typedef Population<individual_type, individual_ptr_type> population_type;
        //! Value type stored in population.
        typedef typename population_type::value_type population_entry_type;
        //! Meta-data type.
        typedef MetaData md_type;
        //! Population initializer type.
        typedef Initializer initializer_type;
        //! Random number generator type.
        typedef RandomNumberGenerator rng_type;
        //! Event handler.
        typedef EventHandler<artificial_life> event_handler_type;
        
        //! Default constructor.
        artificial_life() {
        }
        
        //! Initialize this EA.
        void initialize() {
            _env.initialize(*this);
            _scheduler.initialize(*this);
        } 
        
        //! Generates the initial population.
        void generate_initial_population() {
            initializer_type init;
            init(*this);
        }

        //! Reset the population.
        void reset() {
        }

        //! Advance the epoch of this EA by n updates.
        void advance_epoch(std::size_t n) {
            for( ; n>0; --n) {
                update();
            }
            _events.record_statistics(*this);
            _events.end_of_epoch(*this);
        }
        
        //! Advance this EA by one update.
        void update() {
            _events.record_statistics(*this);
            _scheduler(_population, *this);
            _scheduler.next_update();
            _events.end_of_update(*this);
        }
        
        //! Returns the current update of this EA.
        unsigned long current_update() {
            return _scheduler.current_update();
        }

        //! Perform any needed preselection.
        void preselect(population_type& src) {
        }

        //! Accessor for the random number generator.
        rng_type& rng() { return _rng; }
        
        //! Accessor for the population model object.
        population_type& population() { return _population; }
        
        //! Retrieves this AL's meta-data.
        md_type& md() { return _md; }
        
        //! Retrieves this AL's environment.
        environment_type& env() { return _env; }
        
        //! Retrieves this AL's event handler.
        event_handler_type& events() { return _events; }
        
        //! Retrieves this AL's instruction set architecture.
        isa_type& isa() { return _isa; }
        
        //! Retrieves this AL's task library.
        tasklib_type& tasklib() { return _tasklib; }
        
        //! Retrieves this AL's scheduler.
        scheduler_type& scheduler() { return _scheduler; }
        
    protected:
        rng_type _rng; //!< Random number generator.
        environment_type _env; //!< Environment object.
        scheduler_type _scheduler; //!< Scheduler instance.
        population_type _population; //!< Population instance.
        md_type _md; //!< Meta-data for this evolutionary algorithm instance.
        event_handler_type _events; //!< Event handler.
        isa_type _isa; //!< Instruction set architecture.
        tasklib_type _tasklib; //!< Task library.
        
    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("rng", _rng);
            ar & boost::serialization::make_nvp("environment", _env);
            ar & boost::serialization::make_nvp("population", _population);
            ar & boost::serialization::make_nvp("meta_data", _md);
        }
    };

} // ea

#endif

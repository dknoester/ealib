/* digital_evolution.h 
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
#ifndef _EA_DIGITAL_EVOLUTION_H_
#define _EA_DIGITAL_EVOLUTION_H_

#include <boost/serialization/nvp.hpp>
#include <boost/shared_ptr.hpp>

#include <ea/concepts.h>
#include <ea/configuration.h>
#include <ea/digital_evolution/events.h>
#include <ea/digital_evolution/ancestors.h>
#include <ea/digital_evolution/hardware.h>
#include <ea/digital_evolution/isa.h>
#include <ea/digital_evolution/organism.h>
#include <ea/digital_evolution/schedulers.h>
#include <ea/digital_evolution/replication.h>
#include <ea/digital_evolution/well_mixed.h>
#include <ea/digital_evolution/task_library.h>
#include <ea/ancestors.h>
#include <ea/interface.h>
#include <ea/meta_data.h>
#include <ea/mutation.h>
#include <ea/population.h>
#include <ea/recombination.h>
#include <ea/rng.h>



namespace ea {    
    
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
     that are typically found in an EA.  Thus, the digital_evolution class found
     here.
     
     In general, the design of this class is based on concepts from the Avida 
     platform for digital evolution~\cite{ofria2004}.
     
     In order to preserve as much compatibility between EA and AL components,
     the "organisms" in AL are referred to as "individuals."
     */
	template <
    template <typename> class ConfigurationStrategy,
    template <typename> class Environment=well_mixed,
    typename ReplacementStrategy=first_neighbor,
    template <typename> class Scheduler=weighted_round_robin,
    template <typename> class TaskLibrary=task_library,
    typename Hardware=hardware,
    template <typename> class InstructionSetArchitecture=isa,
	typename MutationOperator=mutation::per_site<mutation::uniform_integer>,
    template <typename> class Individual=organism,
	template <typename, typename> class Population=population,
	template <typename> class EventHandler=alife_event_handler,
	typename MetaData=meta_data,
	typename RandomNumberGenerator=ea::default_rng_type>
    class digital_evolution {
    public:
        //! Configuration object type.
        typedef ConfigurationStrategy<digital_evolution> configuration_type;
        //! Hardware type.
        typedef Hardware hardware_type;
        //! Representation type.
        typedef typename hardware_type::representation_type representation_type;
        //! Scheduler type.
        typedef Scheduler<digital_evolution> scheduler_type;
        //! Scheduler fitness type.
        typedef typename scheduler_type::priority_type priority_type;
        //! Individual type.
        typedef Individual<digital_evolution> individual_type;
        //! Individual pointer type.
        typedef boost::shared_ptr<individual_type> individual_ptr_type;
        //! ISA type.
        typedef InstructionSetArchitecture<digital_evolution> isa_type;
        //! Replacment strategy type.
        typedef ReplacementStrategy replacement_type;
        //! Environment type.
        typedef Environment<digital_evolution> environment_type;
        //! Task library type.
        typedef TaskLibrary<digital_evolution> tasklib_type;
        //! Mutation operator type.
        typedef MutationOperator mutation_operator_type;
        //! Population type.
        typedef Population<individual_type, individual_ptr_type> population_type;
        //! Value type stored in population.
        typedef typename population_type::value_type population_entry_type;
        //! Meta-data type.
        typedef MetaData md_type;
        //! Random number generator type.
        typedef RandomNumberGenerator rng_type;
        //! Event handler.
        typedef EventHandler<digital_evolution> event_handler_type;
        
        //! Default constructor.
        digital_evolution() {
            _configurator.construct(*this);
        }
                
        //! Initialize this EA.
        void initialize() {
            _env.initialize(*this);
            _scheduler.initialize(*this);
            _isa.initialize(*this);
            _configurator.initialize(*this);
        } 
        
        //! Generates the initial population.
        void generate_initial_population() {
            _configurator.initial_population(*this);
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
        
        //! Build an individual from the given representation.
        individual_ptr_type make_individual(const representation_type& r) {
            individual_ptr_type p(new individual_type(r));
            return p;
        }
        
        //! Append individual x to the population and environment.
        void append(individual_ptr_type p) {
            _population.insert(_population.end(), p);
            _env.append(p);
        }
        
        //! Append the range of individuals [f,l) to the population and environment.
        template <typename ForwardIterator>
        void append(ForwardIterator f, ForwardIterator l) {
            _population.insert(_population.end(), f, l);
            _env.append(f, l);
        }
        
        //! (Re-)Place an offspring in the population, if possible.
        void replace(individual_ptr_type parent, individual_ptr_type offspring) {
            replacement_type r;
            std::pair<typename environment_type::iterator, bool> l=r(parent, *this);
            
            if(l.second) {
                _env.replace(l.first, offspring, *this);
                offspring->priority() = parent->priority();
                _population.insert(_population.end(), offspring);
                _events.birth(*offspring, *this);
            }
        }
        
        //! Reset this EA.
        void reset() {
            _configurator.reset(*this);
        }
        
        //! Returns the current update of this EA.
        unsigned long current_update() {
            return _scheduler.current_update();
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
        configuration_type _configurator; //!< Configuration object.

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

#ifndef _EA_ARTIFICIAL_LIFE_H_
#define _EA_ARTIFICIAL_LIFE_H_

#include <boost/serialization/nvp.hpp>
#include <boost/shared_ptr.hpp>

#include <ea/concepts.h>
#include <ea/artificial_life/organism.h>
#include <ea/artificial_life/environment.h>
#include <ea/artificial_life/schedulers.h>
#include <ea/artificial_life/replication.h>
#include <ea/artificial_life/topology.h>
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
    template <typename,typename,typename> class InstructionSet,
    template <typename> class Topology=well_mixed,
    typename ReplacementStrategy=first_neighbor,
    typename Scheduler=round_robin,
	typename MutationOperator=mutation::per_site<mutation::uniform_integer>,
	template <typename,typename,typename> class Environment=environment,
    typename TaskLibrary=task_library,
    template <typename,typename,typename> class Individual=organism,
	template <typename, typename> class Population=population,
	typename Initializer=initialization::random_individual,
	template <typename> class EventHandler=event_handler,
	typename MetaData=meta_data,
	typename RandomNumberGenerator=ea::default_rng_type>
    class artificial_life {
    public:
        //! This evolutionary_algorithm's type.
        typedef artificial_life<Hardware, InstructionSet, Topology, 
        ReplacementStrategy, Scheduler, MutationOperator, Environment,
        TaskLibrary, Individual, Population, Initializer, EventHandler, MetaData, RandomNumberGenerator
        > this_type;
        
        //! Hardware type.
        typedef Hardware hardware_type;
        //! Representation type.
        typedef typename hardware_type::representation_type representation_type;
        //! Scheduler type.
        typedef Scheduler scheduler_type;
        //! Scheduler fitness type.
        typedef typename scheduler_type::priority_type priority_type;
        //! Individual type.
        typedef Individual<representation_type,hardware_type,scheduler_type> individual_type;
        //! Individual pointer type.
        typedef boost::shared_ptr<individual_type> individual_ptr_type;
        //! ISA type.
        typedef InstructionSet<hardware_type,individual_type,this_type> isa_type;
        //! Topology type.
        typedef Topology<this_type> topology_type;
        //! Replacment strategy type.
        typedef ReplacementStrategy replacement_type;
        //! Environment type.
        typedef Environment<topology_type,replacement_type,scheduler_type> environment_type;
        //! Task library type.
        typedef TaskLibrary tasklib_type;
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
        typedef EventHandler<this_type> event_handler_type;
        
        //! Default constructor.
        artificial_life() {
        }
        
        //! Initialize this EA.
        void initialize() {
            _env.initialize(*this);
            _population.initialize(*this);
            _scheduler.initialize(*this);
        }        
        
        //! Advance the epoch of this EA by n updates.
        void advance_epoch(std::size_t n) {
            for( ; n>0; --n) {
                _events.record_statistics(*this);
                _scheduler(_population, *this);
                _scheduler.next_update();
                _events.end_of_update(*this);
            }
            _events.record_statistics(*this);
            _events.end_of_epoch(*this);
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
        
        //! Retrieves this AL's topology.
        topology_type& topo() { return _topo; }
        
        //! Retrieves this AL's scheduler.
        scheduler_type& scheduler() { return _scheduler; }
        
    protected:
        rng_type _rng; //!< Random number generator.
        environment_type _env; //!< Environment object.
        population_type _population; //!< Population instance.
        scheduler_type _scheduler; //!< Scheduler instance.
        md_type _md; //!< Meta-data for this evolutionary algorithm instance.
        event_handler_type _events; //!< Event handler.
        isa_type _isa; //!< Instruction set architecture.
        tasklib_type _tasklib; //!< Task library.
        topology_type _topo; //!< Topology.
        
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

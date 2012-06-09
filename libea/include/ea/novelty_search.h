#ifndef _EA_NOVELTY_SEARCH_H_
#define _EA_NOVELTY_SEARCH_H_


#include <boost/serialization/nvp.hpp>
#include <boost/shared_ptr.hpp>

#include <ea/evolutionary_algorithm.h>
#include <ea/attributes.h>
#include <ea/concepts.h>
#include <ea/generational_models/synchronous.h>
#include <ea/individual.h>
#include <ea/fitness_function.h>
#include <ea/initialization.h>
#include <ea/interface.h>
#include <ea/meta_data.h>
#include <ea/mutation.h>
#include <ea/population.h>
#include <ea/recombination.h>
#include <ea/events.h>
#include <ea/rng.h>
#include <vector>

namespace ea {
    
    template <
	typename Representation,
	typename MutationOperator,
	typename FitnessFunction,
    typename NoveltyMetric,
	typename RecombinationOperator=recombination::two_point_crossover,
	typename GenerationalModel=generational_models::synchronous< >,
	typename Initializer=initialization::complete_population<initialization::random_individual>,
    template <typename> class IndividualAttrs=individual_attributes,
    template <typename,typename,typename> class Individual=individual,
	template <typename,typename> class Population=population,
	template <typename> class EventHandler=event_handler,
	typename MetaData=meta_data,
	typename RandomNumberGenerator=ea::default_rng_type>
	class novelty_search {
    public:
        //! This evolutionary_algorithm's type.
        typedef novelty_search<Representation, MutationOperator, FitnessFunction, NoveltyMetric,
        RecombinationOperator, GenerationalModel, Initializer, IndividualAttrs,
        Individual, Population, EventHandler, MetaData, RandomNumberGenerator
        > ea_type;
        
        //! Representation type.
        typedef Representation representation_type;
        //! Fitness function type.
        typedef FitnessFunction fitness_function_type;
        //! Fitness type.
        typedef typename fitness_function_type::fitness_type fitness_type;
        //! Novelty metric.
        typedef NoveltyMetric novelty_metric_type;
        //! Attributes attached to individuals.
        typedef IndividualAttrs<ea_type> individual_attr_type;
        //! Individual type.
        typedef Individual<representation_type,fitness_type,individual_attr_type> individual_type;
        //! Individual pointer type.
        typedef boost::shared_ptr<individual_type> individual_ptr_type;
        //! Mutation operator type.
        typedef MutationOperator mutation_operator_type;
        //! Crossover operator type.
        typedef RecombinationOperator recombination_operator_type;
        //! Generational model type.
        typedef GenerationalModel generational_model_type;
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
        typedef EventHandler<ea_type> event_handler_type;
        
        //! Default constructor.
        novelty_search() {
        }
        
        //! Initialize this EA.
        void initialize() {
            _fitness_function.initialize(*this);
        }        
        
        //! Advance the epoch of this EA by n updates.
        void advance_epoch(std::size_t n) {
            calculate_fitness(_population.begin(), _population.end(), *this);
            relativize_fitness(_population.begin(), _population.end(), *this);
            for( ; n>0; --n) {
                update();
            }
            _events.record_statistics(*this);
            _events.end_of_epoch(*this);
        }
        
        //! Advance this EA by one update.
        void update() {
            _events.record_statistics(*this);
            _generational_model(_population, *this);
            _generational_model.next_update();
            _events.end_of_update(*this);
        }
        
        //! Retrieve the current update number.
        unsigned long current_update() {
            return _generational_model.current_update();
        }
        
        //! Calculate novelty distance between two individuals.
        template <typename ForwardIterator>
        double novelty_distance(ForwardIterator a, ForwardIterator b) {
            
            double dist = 0.0;
            
            for (int i = 0; i < a.novelty_point().size(); ++i) {
                
                double diff = a.novelty_point()[i] - b.novelty_point()[i];
                
                dist += diff * diff;
            }
            
            return math::sqrt(dist);
        }
        
        //! Relativize fitness values of individuals in the range [f,l).
        template <typename ForwardIterator>
        void relativize(ForwardIterator f, ForwardIterator l) {
            
            vector<double> nearest_neighbors;
            
            for(typename Population::iterator i; i != l; ++i) {
                
                // determine nearest neighbors in phenotype landscape
                nearest_neighbors.clear();
                
                for(typename Population::iterator j = f; j != l; ++j) {
                    
                    if (i != j) {
                        
                        nearest_neighbors.push_back(novelty_distance(i, j));
                    }
                }
                
                for(typename Population::iterator j = _archive.begin(); j != _archive.end(); ++j) {
                    
                    nearest_neighbors.push_back(novelty_distance(i, j));
                }
                
                // sort novelty distances ascending
                std::sort(nearest_neighbors.begin(), nearest_neighbors.end());
                
                // remove elements down to _num_neighbors number of novelty distances
                nearest_neighbors.resize(_num_neighbors);
                
                // average the novelty distances and assign that as this individual's novelty fitness
                double avg = 0.0;
                
                for (vector<double>::iterator j = nearest_neighbors.begin(); j != nearest_neighbors.end(); ++j) {
                    
                    avg += *j;
                }
                
                f.novelty_fitness() = avg / _num_neighbors;
                
                // reassign novelty fitness to fitness so novelty is used in GA selection
                f.objective_fitness() = f.fitness();
                f.fitness() = f.novelty_fitness();
            }
            
            // add highly novel individuals to the archive
            int archive_add_count = 0;
            
            for(typename Population::iterator i = f; i != l; ++i) {
                
                if(i.novelty_fitness() > _novelty_threshold) {
                    
                    _archive.append(i);
                    ++archive_add_count;
                }
            }
            
            // adjust the archive threshold, if necessary
            if (archive_add_count > 3) {
                _novelty_threshold *= 1.1;
            }
            else if (archive_add_count == 0) {
                _novelty_threshold *= 0.9;
            }
        }
        
        //! Retrieve the random number generator.
        rng_type& rng() { return _rng; }
        
        //! Retrieve the population.
        population_type& population() { return _population; }
        
        //! Retrieve the archive of novel individuals.
        population_type& archive() { return _archive; }
        
        //! Retrieve this EA's meta-data.
        md_type& md() { return _md; }
        
        //! Retrieve the fitness function.
        fitness_function_type& fitness_function() { return _fitness_function; }
        
        //! Retrieve the generational model object.
        generational_model_type& generational_model() { return _generational_model; }
        
        //! Retrieve the event handler.
        event_handler_type& events() { return _events; }
        
    protected:
        rng_type _rng;                                  //!< Random number generator.
        fitness_function_type _fitness_function;        //!< Fitness function object.
        population_type _population;                    //!< Population instance.
        md_type _md;                                    //!< Meta-data for this evolutionary algorithm instance.
        generational_model_type _generational_model;    //!< Generational model instance.
        event_handler_type _events;                     //!< Event handler.
        population_type _archive;                       //!< Archive of novel individuals.
        double _novelty_threshold = 6.0;                //!< Threshold for admission into archive.
        int _num_neighbors = 15;                        //!< Number of nearest neighbors to take into account when calculating novelty.
        
    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("rng", _rng);
            ar & boost::serialization::make_nvp("fitness_function", _fitness_function);
            ar & boost::serialization::make_nvp("population", _population);
            ar & boost::serialization::make_nvp("archive", _archive);
            ar & boost::serialization::make_nvp("generational_model", _generational_model);
            ar & boost::serialization::make_nvp("meta_data", _md);
        }
    };
    
} // ea

#endif

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
    
    /*! Definition of an individual for novelty search.
     
     This class extends the standard individual by adding objective, novelty, 
     and novelty point member variables.
	 */
	template <typename Representation, typename FitnessType, typename Attributes>
	class novelty_individual : public individual <Representation, FitnessType, Attributes> {
	public:
        
        typedef Representation representation_type;
		typedef FitnessType fitness_type;
        typedef Attributes attr_type;
        typedef individual<representation_type, fitness_type, attr_type> base_type;
        typedef novelty_individual<representation_type, fitness_type, attr_type> individual_type;
		
		//! Constructor.
		novelty_individual() : base_type() {
		}
        
		//! Constructor that builds a novelty individual from a representation.
		novelty_individual(const representation_type& r) : base_type(r) {
		}
        
        //! Copy constructor.
        novelty_individual(const novelty_individual& that) : base_type(that) {
            _objective_fitness = that._objective_fitness;
            _novelty_fitness = that._novelty_fitness;
            _novelty_point = that._novelty_point;
        }
        
        //! Assignment operator.
        novelty_individual& operator=(const novelty_individual& that) {
            if(this != &that) {
                base_type::operator=(that);
                
                _objective_fitness = that._objective_fitness;
                _novelty_fitness = that._novelty_fitness;
                _novelty_point = that._novelty_point;
            }
            return *this;
        }
        
        //! Destructor.
        virtual ~novelty_individual() {
        }
        
        //! Retrieve this individual's objective fitness.
		fitness_type& objective_fitness() { return _objective_fitness; }
        
        //! Retrieve this individual's novelty fitness.
		fitness_type& novelty_fitness() { return _novelty_fitness; }
        
        //! Retrieve this individual's novelty point.
        std::vector<double> novelty_point() { return _novelty_point; }
		
        //! Retrieve this individual's fitness (const-qualified).
		const fitness_type& objective_fitness() const { return _objective_fitness; }
        
        //! Retrieve this individual's fitness (const-qualified).
		const fitness_type& novelty_fitness() const { return _novelty_fitness; }
        
	protected:
        fitness_type _objective_fitness; //!< This individual's objective fitness.
        fitness_type _novelty_fitness; //!< This individual's novelty fitness.
        std::vector<double> _novelty_point; //!< This individual's location in phenotype space.
        
	private:
		friend class boost::serialization::access;
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version) { 
            ar & boost::serialization::make_nvp("base_individual", boost::serialization::base_object<base_type>(*this));
            ar & boost::serialization::make_nvp("objective_fitness", _objective_fitness);
            ar & boost::serialization::make_nvp("novelty_fitness", _novelty_fitness);
            ar & boost::serialization::make_nvp("novelty_point", _novelty_point);
		}
	};
    
    
    /*! Novelty search evolutionary algorithm.
     
     TODO: Explain how NS works, provide cite to paper.
     */
    template <
	typename Representation,
	typename MutationOperator,
	typename FitnessFunction,
    typename NoveltyMetric,
	typename RecombinationOperator=recombination::two_point_crossover,
	typename GenerationalModel=generational_models::synchronous< >,
	typename Initializer=initialization::complete_population<initialization::random_individual>,
    template <typename> class IndividualAttrs=individual_attributes,
    template <typename,typename,typename> class Individual=novelty_individual,
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
        
        //! Relativize fitness values of individuals in the range [f,l).
        template <typename ForwardIterator>
        void relativize(ForwardIterator f, ForwardIterator l) {
            
            std::vector<double> nearest_neighbors(_archive.size() + std::distance(f, l));
            int archive_add_count = 0;
            double fitness_sum = 0.0;
            
            for(ForwardIterator i=f; i!=l; ++i) {
                nearest_neighbors.clear();
                
                for(ForwardIterator j=f; j!=l; ++j) {
                    if (i != j) {
                        nearest_neighbors.push_back(algorithm::vdist(ind(i, *this).novelty_point().begin(),
                                                                     ind(i, *this).novelty_point().end(),
                                                                     ind(j, *this).novelty_point().begin(),
                                                                     ind(j, *this).novelty_point().end()));
                    }
                }
                
                for(typename population_type::iterator j=_archive.begin(); j!=_archive.end(); ++j) {
                    nearest_neighbors.push_back(algorithm::vdist(ind(i, *this).novelty_point().begin(),
                                                                 ind(i, *this).novelty_point().end(),
                                                                 ind(j, *this).novelty_point().begin(),
                                                                 ind(j, *this).novelty_point().end()));
                }
                
                // sort novelty distances ascending
                std::sort(nearest_neighbors.begin(), nearest_neighbors.end());
                
                ind(i, *this).novelty_fitness() = algorithm::vmean(nearest_neighbors.begin(),
                                                                   nearest_neighbors.begin() + get<NOVELTY_NEIGHBORHOOD_SIZE>(*this),
                                                                   0.0);
                
                // reassign novelty fitness to fitness so novelty is used in GA selection
                ind(i, *this).objective_fitness() = ind(i, *this).fitness();
                ind(i, *this).fitness() = ind(i, *this).novelty_fitness();
                
                // keep a running sum of the novelty fitness
                fitness_sum += ind(i, *this).fitness();
                
                // add highly novel individuals to the archive
                if(ind(i, *this).novelty_fitness() > get<NOVELTY_THRESHOLD>(*this)) {
                    _archive.append(i);
                    ++archive_add_count;
                }
            }
            
            // adjust the archive threshold, if necessary
            if (archive_add_count > 3) {
                scale<NOVELTY_THRESHOLD>(1.1, *this);
            } else if (archive_add_count == 0) {
                scale<NOVELTY_THRESHOLD>(0.9, *this);
            }
            
            // if all novelty fitnesses are 0.0, reset all of them to 1.0 so selection doesn't break
            if (fitness_sum == 0.0) {
                
                for(ForwardIterator i=f; i!=l; ++i) {
                    ind(i, *this).fitness() = 1.0;
                }
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

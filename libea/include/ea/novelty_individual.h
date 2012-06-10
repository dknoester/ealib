#ifndef _EA_NOVELTY_INDIVIDUAL_H_
#define _EA_NOVELTY_INDIVIDUAL_H_

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/shared_ptr.hpp>

#include <fstream>
#include <vector>

#include <ea/individual.h>
#include <ea/meta_data.h>

namespace ea {
    
	/*! Definition of a novelty individual.
	 
	 Individuals within ealib are four things:
	 1) A container for a representation
	 2) A container for fitness (objective, novelty, and novelty point)
	 3) A container for meta data
     4) A container for attributes
     
	 */
	template <typename Representation, typename FitnessType, typename Attributes>
	class novelty_individual : individual <Representation, FitnessType, Attributes> {
	public:
        
        typedef Representation representation_type;
		typedef FitnessType fitness_type;
        typedef Attributes attributes;
        
        typedef individual<representation_type, fitness_type, attributes> base_type;
        typedef lod_individual<representation_type, fitness_type, attributes> individual_type;
		
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
        
		/* These enable serialization and de-serialization of individuals.
		 This would be easy, except for the fact that we can't round-trip NaNs.  So,
		 we store a flag instead of fitness in that case.
         */ 
		friend class boost::serialization::access;
		template<class Archive>
		void save(Archive & ar, const unsigned int version) const {
            
            //! TODO: what is base_object?
            ar & boost::serialization::base_object<base_type>(*this);
            ar & boost::serialization::make_nvp("objective_fitness", _objective_fitness);
            ar & boost::serialization::make_nvp("novelty_fitness", _novelty_fitness);
            ar & boost::serialization::make_nvp("novelty_point", _novelty_point);
		}
		
		template<class Archive>
		void load(Archive & ar, const unsigned int version) {
            ar & boost::serialization::base_object<base_type>(*this);
            ar & boost::serialization::make_nvp("objective_fitness", _objective_fitness);
            ar & boost::serialization::make_nvp("novelty_fitness", _novelty_fitness);
            ar & boost::serialization::make_nvp("novelty_point", _novelty_point);
		}
        
		BOOST_SERIALIZATION_SPLIT_MEMBER();
	};
    
} // ea

#endif

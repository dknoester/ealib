#ifndef _EA_NOVELTY_INDIVIDUAL_H_
#define _EA_NOVELTY_INDIVIDUAL_H_

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/shared_ptr.hpp>
#include <fstream>
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
	class novelty_individual : individual {
	public:
		
		//! Constructor.
		novelty_individual() : individual() {
		}
        
		//! Constructor that builds a novelty individual from a representation.
		novelty_individual(const representation_type& r) : individual(r) {
		}
        
        //! Copy constructor.
        novelty_individual(const novelty_individual& that) : individual(that) {
            _objective_fitness = that._objective_fitness;
            _novelty_fitness = that._novelty_fitness;
            _novelty_point = that._novelty_point;
        }
        
        //! Assignment operator.
        novelty_individual& operator=(const novelty_individual& that) {
            //! TODO: rework so this inherits from base individual.
            if(this != &that) {
                _name = that._name;
                _generation = that._generation;
                _update = that._update;
                _fitness = that._fitness;
                _objective_fitness = that._objective_fitness;
                _novelty_fitness = that._novelty_fitness;
                _novelty_point = that._novelty_point;
                _repr = that._repr;
                _md = that._md;
                _attr = that._attr;
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
        
        //! TODO: Change these private methods to protected so they can be used by subclasses of individual?
        
		/* These enable serialization and de-serialization of individuals.
		 This would be easy, except for the fact that we can't round-trip NaNs.  So,
		 we store a flag instead of fitness in that case.
         */ 
		friend class boost::serialization::access;
		template<class Archive>
		void save(Archive & ar, const unsigned int version) const {
            ar & boost::serialization::make_nvp("name", _name);
            ar & boost::serialization::make_nvp("generation", _generation);
			bool null_fitness=_fitness.is_null();
			ar & BOOST_SERIALIZATION_NVP(null_fitness);
			if(!null_fitness) {
				ar & boost::serialization::make_nvp("fitness", _fitness);
			}
			ar & boost::serialization::make_nvp("representation", _repr);
            ar & boost::serialization::make_nvp("meta_data", _md);
            ar & boost::serialization::make_nvp("update", _update);
            ar & boost::serialization::make_nvp("attributes", _attr);
		}
		
		template<class Archive>
		void load(Archive & ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("name", _name);
            ar & boost::serialization::make_nvp("generation", _generation);
			bool null_fitness=true;
			ar & BOOST_SERIALIZATION_NVP(null_fitness);
			if(null_fitness) {
                _fitness.nullify();
			} else {
				ar & boost::serialization::make_nvp("fitness", _fitness);
			}
			ar & boost::serialization::make_nvp("representation", _repr);
            ar & boost::serialization::make_nvp("meta_data", _md);
            ar & boost::serialization::make_nvp("update", _update);
            ar & boost::serialization::make_nvp("attributes", _attr);
		}
        
		BOOST_SERIALIZATION_SPLIT_MEMBER();
	};	
    
    /*! Serialize an individual.
     */
    template <typename EA>
    void individual_save(std::ostream& out, typename EA::individual_type& ind, EA& ea) {
        boost::archive::xml_oarchive oa(out);
        oa << boost::serialization::make_nvp("novelty_individual",ind);
    }
    
    
	/*! Load a previously serialized individual.
     */
	template <typename EA>
    typename EA::individual_type individual_load(std::istream& in, EA& ea) {
        typename EA::individual_type ind;
		boost::archive::xml_iarchive ia(in);
        ia >> boost::serialization::make_nvp("novelty_individual", ind);
        return ind;
	}
	
    //! TODO: Is the below function okay to omit?
    
    template <typename EA>
    typename EA::individual_type individual_load(const std::string& fname, EA& ea) {
        std::ifstream ifs(fname.c_str());
        return individual_load(ifs, ea);
    }
    
} // ea

#endif

/* individual.h
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
#ifndef _EA_INDIVIDUAL_H_
#define _EA_INDIVIDUAL_H_

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_member.hpp>
#include <ea/meta_data.h>

namespace ea {

	/*! Definition of an individual.
	 
	 Individuals within ealib are four things:
	 1) A container for a representation
	 2) A container for fitness
	 3) A container for meta data
     4) A container for attributes

	 */
	template <typename Representation, typename FitnessType, typename Attributes>
	class individual {
	public:
		typedef Representation representation_type;
		typedef FitnessType fitness_type;
        typedef Attributes attr_type;
		
		//! Constructor.
		individual() : _name(0), _generation(0.0), _update(0) {
		}
        
		//! Constructor that builds an individual from a representation.
		individual(const representation_type& r) : _name(0), _generation(0.0), _update(0), _repr(r) {
		}
        
        //! Copy constructor.
        individual(const individual& that) {
            _name = that._name;
            _generation = that._generation;
            _update = that._update;
            _fitness = that._fitness;
            _repr = that._repr;
            _md = that._md;
            _attr = that._attr;
        }
        
        //! Assignment operator.
        individual& operator=(const individual& that) {
            if(this != &that) {
                _name = that._name;
                _generation = that._generation;
                _update = that._update;
                _fitness = that._fitness;
                _repr = that._repr;
                _md = that._md;
                _attr = that._attr;
            }
            return *this;
        }
        
        //! Destructor.
        virtual ~individual() {
        }
        
        //! Retrieve this individual's name.
        long& name() { return _name; }
		
        //! Retrieve this individual's name (const-qualified).
        const long& name() const { return _name; }

        //! Retrieve this individual's generation.
        double& generation() { return _generation; }

        //! Retrieve this individual's generation (const-qualified).
        const double& generation() const { return _generation; }
        
        //! Retrieve this individual's update.
        long& update() { return _update; }

        //! Retrieve this individual's update (const-qualified).
        const long& update() const { return _update; }

        //! Cast this individual to its fitness.
		operator fitness_type() { return _fitness; }

		//! Retrieve this individual's fitness.
		fitness_type& fitness() { return _fitness; }
		
		//! Retrieve this individual's fitness (const-qualified).
		const fitness_type& fitness() const { return _fitness; }
		
		//! Retrieve this individual's representation.
		representation_type& repr() { return _repr; }
        
		//! Retrieve this individual's representation (const-qualified).
		const representation_type& repr() const { return _repr; }
        
        //! Retrieve this individual's meta data.
        meta_data& md() { return _md; }

        //! Retrieve this individual's meta data (const-qualified).
        const meta_data& md() const { return _md; }
        
        //! Retrieve this individual's attributes.
        attr_type& attr() { return _attr; }
        
        //! Retrieve this individual's attributes (const-qualified).
        const attr_type& attr() const { return _attr; }

	protected:
        long _name; //!< Name (id number) of this individual.
        double _generation; //!< Generation of this individual.
        long _update; //!< Update at which this individual was born.
		fitness_type _fitness; //!< This individual's fitness. This is the fitness that the GA uses for selection.
		representation_type _repr; //!< This individual's representation.
        meta_data _md; //!< This individual's meta data.
        attr_type _attr; //!< This individual's attributes.
        
	private:
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

//	/*! Compare individuals by their fitness.
//	 */
//	template <typename Representation, typename FitnessFunction>
//	bool operator<(const individual<Representation,FitnessFunction>& a, const individual<Representation,FitnessFunction>& b) {
//		return a.fitness() < b.fitness();
//	}
    
    
    /*! Serialize an individual.
     */
//    template <typename EA>
//    void individual_save(std::ostream& out, typename EA::individual_type& ind, EA& ea) {
//        boost::archive::xml_oarchive oa(out);
//        oa << boost::serialization::make_nvp("individual",ind);
//    }
//    
//    
//	/*! Load a previously serialized individual.
//     */
//	template <typename EA>
//    typename EA::individual_type individual_load(std::istream& in, EA& ea) {
//        typename EA::individual_type ind;
//		boost::archive::xml_iarchive ia(in);
//        ia >> boost::serialization::make_nvp("individual", ind);
//        return ind;
//	}
//	
//    template <typename EA>
//    typename EA::individual_type individual_load(const std::string& fname, EA& ea) {
//        std::ifstream ifs(fname.c_str());
//        return individual_load(ifs, ea);
//    }

} // ea

#endif

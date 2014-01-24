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

#include <ea/meta_data.h>
#include <ea/traits.h>

namespace ealib {
    
    /*! Individual within an evolutionary algorithm.
	 */
	template
    < typename Representation
    , typename FitnessFunction
    , typename Phenotype=Representation
    , typename Encoding=directS
    , template <typename> class Traits=default_traits
    > class individual {
	public:
        //! Representation type; the "genome."
		typedef Representation representation_type;
        //! Fitness function type.
        typedef FitnessFunction fitness_function_type;
        //! Fitness value type for this individual.
        typedef typename FitnessFunction::fitness_type fitness_type;
        //! Phenotype for this individual.
        typedef Phenotype phenotype_type;
        //! Encoding of this individual.
        typedef Encoding encoding_type;
        //! Pointer to this individual.
        typedef boost::shared_ptr<individual> individual_ptr_type;
        //! Traits for this individual.
        typedef Traits<individual> traits_type;
        //! Phenotype pointer type.
        typedef typename traits_type::phenotype_ptr_type phenotype_ptr_type;
        //! Meta-data type.
        typedef meta_data md_type;
        
        //! Constructor.
		individual() {
		}
        
		//! Constructor that builds an individual from a representation.
		individual(const representation_type& r) : _repr(r) {
		}
        
        //! Copy constructor.
        individual(const individual& that) {
            _repr = that._repr;
            _md = that._md;
            _traits = that._traits;
        }
        
        //! Assignment operator.
        individual& operator=(const individual& that) {
            if(this != &that) {
                _repr = that._repr;
                _md = that._md;
                _traits = that._traits;
            }
            return *this;
        }
        
		//! Returns this individual's representation.
		representation_type& repr() { return _repr; }
        
		//! Returns this individual's representation (const-qualified).
		const representation_type& repr() const { return _repr; }
        
        //! Returns this individual's fitness.
        fitness_type& fitness() { return _fitness; }

        //! Returns this individual's fitness (const-qualified).
        const fitness_type& fitness() const { return _fitness; }

        //! Returns this individual's meta data.
        meta_data& md() { return _md; }
        
        //! Returns this individual's meta data (const-qualified).
        const meta_data& md() const { return _md; }
        
        //! Returns this individual's traits.
        traits_type& traits() { return _traits; }
        
        //! Returns this individual's traits (const-qualified).
        const traits_type& traits() const { return _traits; }

    protected:
        representation_type _repr; //!< This individual's representation.
        fitness_type _fitness; //!< This individual's fitness.
        meta_data _md; //!< This individual's meta data.
        traits_type _traits; //!< This individual's traits.
        
    private:
        friend class boost::serialization::access;
        
        //! Serialize this individual.
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("representation", _repr);
            ar & boost::serialization::make_nvp("fitness", _fitness);
            ar & boost::serialization::make_nvp("meta_data", _md);
        }
    };
    
} // ealib

#endif

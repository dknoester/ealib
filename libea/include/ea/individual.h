/* individual.h
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
    , typename Traits
    > class individual {
	public:
		typedef Representation representation_type;
        typedef typename representation_type::genome_type genome_type;
        typedef typename representation_type::phenotype_type phenotype_type;
        typedef typename representation_type::encoding_type encoding_type;
        typedef Traits traits_type;
        typedef meta_data md_type;
        
        //! Constructor.
		individual() {
		}
        
		//! Constructor that builds an individual from a genome.
		individual(const genome_type& g) : _repr(g) {
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
        
		//! Returns this individual's genome.
		genome_type& genome() { return _repr.genome(); }
        
		//! Returns this individual's genome (const-qualified).
		const genome_type& genome() const { return _repr.genome(); }
        
        //! Returns this individual's traits.
        traits_type& traits() { return _traits; }
        
        //! Returns this individual's traits (const-qualified).
        const traits_type& traits() const { return _traits; }

        //! Returns this individual's meta data.
        meta_data& md() { return _md; }
        
        //! Returns this individual's meta data (const-qualified).
        const meta_data& md() const { return _md; }

    protected:
        representation_type _repr; //!< This individual's representation.
        traits_type _traits; //!< This individual's traits.
        meta_data _md; //!< This individual's meta data.
        
    private:
        friend class boost::serialization::access;
        
        //! Serialize this individual.
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("representation", _repr);
            ar & boost::serialization::make_nvp("traits", _traits);
            ar & boost::serialization::make_nvp("meta_data", _md);
        }
    };
    
} // ealib

#endif

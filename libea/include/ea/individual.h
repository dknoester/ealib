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
#include <sstream>
#include <ea/meta_data.h>
#include <ea/phenotype.h>
#include <ea/traits.h>

namespace ealib {
    
    LIBEA_MD_DECL(IND_NAME, "individual.name", long);
    LIBEA_MD_DECL(IND_GENERATION, "individual.generation", double);
    LIBEA_MD_DECL(IND_BIRTH_UPDATE, "individual.birth_update", long);
    
    /*! Individual within an evolutionary algorithm.
	 */
	template
    < typename Representation
    , typename FitnessFunction
    , typename Phenotype=Representation
    , typename Encoding=directS
    , template <typename> class Traits=ealib::default_traits
    > class individual {
	public:
        //! Representation type; the "genome."
		typedef Representation representation_type;
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
        
        //! Destructor.
        virtual ~individual() {
        }
        
		//! Retrieve this individual's representation.
		representation_type& repr() { return _repr; }
        
		//! Retrieve this individual's representation (const-qualified).
		const representation_type& repr() const { return _repr; }
        
        //! Retrieve this individual's meta data.
        meta_data& md() { return _md; }
        
        //! Retrieve this individual's meta data (const-qualified).
        const meta_data& md() const { return _md; }
        
        //! Retrieve this individual's attributes.
        traits_type& traits() { return _traits; }
        
        //! Retrieve this individual's attributes (const-qualified).
        const traits_type& traits() const { return _traits; }
        
        //! Cast this individual to a std::string.
        operator std::string() {
            std::ostringstream s;
            s << "individual=" << get<IND_NAME>(*this) << ", generation=" << get<IND_GENERATION>(*this);
            return s.str();
        }
        
    protected:
        representation_type _repr; //!< This individual's representation.
        meta_data _md; //!< This individual's meta data.
        traits_type _traits; //!< This individual's traits.
        
    private:
        friend class boost::serialization::access;
        
        //! Serialize this individual.
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("representation", _repr);
            ar & boost::serialization::make_nvp("meta_data", _md);
            ar & boost::serialization::make_nvp("traits", _traits);
        }
    };
    
} // ealib

#endif

/* subpopulation.h
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
#ifndef _EA_SUBPOPULATION_H_
#define _EA_SUBPOPULATION_H_

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_member.hpp>

namespace ealib {
    
    /*! Subpopulation class for metapopulation EAs.
     
     Subpopulation simply extends an EA template parameter to provide a traits
     object.  This traits object can be used, e.g., to store a fitness value or
     founder individual.
     */
	template
    < typename EA
    , typename Traits
    > class subpopulation : public EA {
    public:
        typedef EA parent;
        typedef Traits traits_type;

        //! Constructor.
        subpopulation() {
        }
        
        //! Copy constructor.
        subpopulation(const subpopulation& sub) : parent(sub), _traits(sub._traits) {
        }
        
        //! Returns this subpopulation's traits.
        traits_type& traits() { return _traits; }
        
        //! Returns this individual's traits (const-qualified).
        const traits_type& traits() const { return _traits; }
        
    protected:
        traits_type _traits; //!< This subpopulation's traits.
        
    private:
        //! Serialization.
        friend class boost::serialization::access;
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("ea", boost::serialization::base_object<parent>(*this));
            ar & boost::serialization::make_nvp("traits", _traits);
		}
    };

} // ealib

#endif

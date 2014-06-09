/* subpopulation_founder.h
 *
 * This file is part of EALib.
 *
 * Copyright 2014 David B. Knoester, Heather J. Goldsby.
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
#ifndef _EA_SUBPOPULATION_FOUNDER_H_
#define _EA_SUBPOPULATION_FOUNDER_H_

#include <ea/line_of_descent.h>

namespace ealib {
    
    /*! Conceptually, a subpopulation founder is whatever is needed to recover
     the exact intial condition of a subpopulation EA.
     
     The subpopulation_founder_event (below) is called upon inheritance to store
     an exact copy of the offspring in the subpopulation_founder_trait.
     
     Subpopulation founders strongly imply that we're also tracking the line of
     descent, so we handle that here, too.  To enable LoD tracking, use
     lod_event.
     */
    template <typename T>
    struct subpopulation_founder_trait : lod_trait<T> {
        typedef lod_trait<T> parent;
        typedef typename T::individual_type individual_type;
        typedef typename T::individual_ptr_type individual_ptr_type;
        
        //! Returns a pointer to the subpopulation founder.
        individual_ptr_type founder() { return _founder; }

        individual_ptr_type _founder; //!< Founder.
        
        //! Serialize this trait.
        template<class Archive>
		void save(Archive & ar, const unsigned int version) const {
            ar & boost::serialization::make_nvp("lod_trait", boost::serialization::base_object<parent>(*this));
            bool has_founder = (_founder != 0);
            ar & boost::serialization::make_nvp("has_founder", has_founder);
            if(has_founder) {
                ar & boost::serialization::make_nvp("founder", *_founder);
            }
		}
		
        //! Serialize this trait.
		template<class Archive>
		void load(Archive & ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("lod_trait", boost::serialization::base_object<parent>(*this));
            bool has_founder=false;
            ar & boost::serialization::make_nvp("has_founder", has_founder);
            if(has_founder) {
                _founder.reset(new individual_type());
                ar & boost::serialization::make_nvp("founder", *_founder);
            }
		}
        
		BOOST_SERIALIZATION_SPLIT_MEMBER();
    };

   
    //! Convenience method to access the founder of a subpopulation.
    template <typename SEA>
    SEA& founder(SEA& sea) {
        return *sea.traits().founder();
    }
    
    
    //! Stores an exact copy of an offspring upon inheritance to create a founder.
    template <typename MEA>
    struct subpopulation_founder_event : inheritance_event<MEA> {
        
        //! Constructor.
        subpopulation_founder_event(MEA& mea) : inheritance_event<MEA>(mea) {
        }
        
        //! Destructor.
        virtual ~subpopulation_founder_event() {
        }
        
        //! Called for every inheritance event.
        virtual void operator()(typename MEA::population_type& parents,
                                typename MEA::individual_type& offspring,
                                MEA& mea) {
            offspring.traits().founder().reset(new typename MEA::individual_type(offspring));
        }
    };

} // ealib

#endif


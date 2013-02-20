/* task_library.h
 *
 * This file is part of EALib.
 *
 * Copyright 2012 David B. Knoester, Heather J. Goldsby.
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
#ifndef _EA_DIGITAL_EVOLUTION_SUBPOPULATION_FOUNDER_H_
#define _EA_DIGITAL_EVOLUTION_SUBPOPULATION_FOUNDER_H_

#include <ea/digital_evolution.h>
#include <ea/digital_evolution/hardware.h>

namespace ea {
    
    template <typename EA>
    class population_founder : public EA {
    public:
        typedef EA base_type;
        typedef typename base_type::individual_type founder_type;
        typedef typename base_type::individual_ptr_type founder_ptr_type;
//        typedef typename base_type::individual_attr_tye
        
        //! Constructor.
        population_founder() : base_type() {
        }
        
        //! Copy constructor.
        population_founder(const population_founder& that) : base_type(that) {
            _founder = that._founder;
        }
        
        //! Assignment operator.
        population_founder& operator=(const population_founder& that) {
            if(this != &that) {
                base_type::operator=(that);
                _founder = that._founder;
            }
            return *this;
        }
        
        //! Destructor.
        virtual ~population_founder() {
        }
        
        founder_type& founder() { return _founder; }
        
    protected:
        founder_type _founder;
        
    private:
        friend class boost::serialization::access;
        
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("founder", _founder);
            ar & boost::serialization::make_nvp("individual", boost::serialization::base_object<base_type>(*this));
        }
    };
    
    
    /*! Chains together offspring and their parents, called for every inheritance event.
     */
    template <typename EA>
    struct founder_event : inheritance_event<EA> {
        
        //! Constructor.
        founder_event(EA& ea) : inheritance_event<EA>(ea) {
        }
        
        //! Destructor.
        virtual ~founder_event() {
        }
        
        //! Called for every inheritance event.
        virtual void operator()(typename EA::population_type& parents,
                                typename EA::individual_type& offspring,
                                EA& ea) {
            offspring.founder() = *offspring.population().front();
        }
    };
}

#endif


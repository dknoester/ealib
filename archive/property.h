/* property.h
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
#ifndef _EA_PROPERTY_H_
#define _EA_PROPERTY_H_

#include <boost/property_tree/ptree.hpp>
#include <map>
#include <string>
#include <ea/exceptions.h>

namespace ealib {
    
    /* WARNING:
     
     This code is in progress, and is likely to change dramatically.
     
     */
    
    //! Abstract property type.
    struct abstract_property {
        virtual void get(const string& name, ptree& pt) = 0;
    };

    /*! Property class.
     
     Properties are used to control configurable aspects of an evolutionary
     algorithm, e.g., population size, mutation rate, etc.  Each component in an
     EA has its own specific properties that must be set prior to running, and
     this class is part of a type-safe way to translate between values found in
     configuration files, command lines, and property values.
     */
    template <typename T>
    struct property : public abstract_property {
        typedef T value_type; //!< Type of this property.

        T _val; //!< Value of this property.
        bool _set; //!< Whether a value has been assigned to this property.

        //! Constructor.
        property() : _set(false) {
        }
        
        //! Converting assignment operator.
        property& operator=(const T& t) {
            _val = t;
            _set = true;
            return *this;
        }

        //! Cast operator.
        inline operator T() {
            if(!_set) {
                throw uninitialized_meta_data_exception("");
            }
            return _val;
        }
        
        //! Load this property (with "name") from a ptree instance.
        void get(const string& name, ptree& pt) {
            _val = pt.get<value_type>(name);
            _set = true;
        }
    };

    /*! Class that ties a series of strings (keys) to property instances.
     */
    struct property_tie {
        typedef map<std::string,abstract_property*> map_type;
        
        //! Tie a string to a property.
        pmap& tie(const std::string& s, abstract_property* ap) {
            _map[s] = ap;
            return *this;
        }
        
        //! Loads all tied properties from the given property tree.
        void load(ptree& pt) {
            for(map_type::iterator i=_map.begin(); i!=_map.end(); ++i) {
                i->second->get(i->first, pt);
            }
        }
        
        map_type _map;
    };
    
} // ealib

#endif

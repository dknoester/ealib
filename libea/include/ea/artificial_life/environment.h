/* artificial_life/environment.h 
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

#ifndef _EA_ARTIFICIAL_LIFE_ENVIRONMENT_H_
#define _EA_ARTIFICIAL_LIFE_ENVIRONMENT_H_

#include <boost/serialization/nvp.hpp>


namespace ea {
    
    /*! Abstract environment type.
     */
    template <typename Topology, typename ReplacementStrategy, typename Scheduler>
    class environment {
    public:
        typedef Topology topology_type; //!< Type for this environment's topology.
        
        //! Initialize this environment.
        template <typename AL>
        void initialize(AL& al) {
        };
        
        //! Read from the environment.
        template <typename Organism>
        int read(Organism& org) {
            return 0;
        }

    protected:
        topology_type _topo; //!< This environment's topology.
        
    private:
        friend class boost::serialization::access;
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("topology", _topo);
        }
    };

} // ea

#endif

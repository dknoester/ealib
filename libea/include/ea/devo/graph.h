/* graph.h
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

// This is old code from the ealife project; revisit when / if we explore grouping again.

//
///*! Group-tracking spatial environment.
// */
//template <typename EA>
//struct grouping : spatial<EA> {
//    typedef spatial<EA> base_type;
//    typedef typename EA::individual_ptr_type individual_ptr_type;
//    
//    //! Properties for vertices in the group graph.
//    struct grouping_vertex_properties {
//        individual_ptr_type p;
//    };
//    
//    typedef boost::adjacency_list<boost::setS, boost::vecS, boost::undirectedS, grouping_vertex_properties> graph_type;
//    typedef std::map<long,typename graph_type::vertex_descriptor> nv_map_type;
//    
//    //! Constructor.
//    grouping() {
//    }
//    
//    //! Destructor.
//    virtual ~grouping() {
//    }
//    
//    //! Initialize this environment.
//    void initialize(EA& ea) {
//        base_type::initialize(ea);
//        _inheritance = ea.events().inheritance.connect(boost::bind(&grouping::inheritance, this, _1, _2, _3));
//        _death =  ea.events().death.connect(boost::bind(&grouping::death, this, _1, _2));
//    }
//    
//    //! Called when an offspring inherits from parents.
//    void inheritance(typename EA::population_type& parents,
//                     typename EA::individual_type& offspring,
//                     EA& ea) {
//    }
//    
//    //! Called when an individual dies.
//    void death(typename EA::individual_type& individual, EA& ea) {
//    }
//    
//    graph_type _g; //!< Group graph.
//    nv_map_type _map; //!< Map of individual's name to vertex in the group graph.
//    boost::signals::scoped_connection _inheritance; //<! Connection to inheritance event.
//    boost::signals::scoped_connection _death; //!< Connection to death event.
//};
//

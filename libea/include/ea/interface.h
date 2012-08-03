/* interface.h
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
#ifndef _EA_INTERFACE_H_
#define _EA_INTERFACE_H_

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <algorithm>
#include <iterator>
#include <ea/concepts.h>
#include <ea/meta_data.h>
#include <ea/generators.h>


/*! The templated methods here define the interface to various pieces of an evolutionary
 algorithm.  The idea is that these methods are the public interface, hiding the
 specifics of a given evolutionary algorithm from users.
 */
namespace ea {
    
  
    //! Retrieve a reference to an individual given a population iterator.
    template <typename EA>
    typename EA::individual_type& ind(typename EA::population_type::iterator i, EA& ea) {
        BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
        return ea.population().ind(i);
    }
    
    //! Retrieve a const reference to an individual given a population const_iterator.
    template <typename EA>
    const typename EA::individual_type& ind(typename EA::population_type::const_iterator i, EA& ea) {
        BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
        return ea.population().ind(i);
    }
    
    //! Retrieve a reference to an individual given an individual pointer.
    template <typename EA>
    typename EA::individual_type& ind(typename EA::individual_ptr_type p, EA& ea) {
        BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
        return *p;
    }
    
    //! Retrieve a reference to an individual's attributes given a population iterator.
    template <typename EA>
    typename EA::individual_attr_type& attr(typename EA::population_type::iterator i, EA& ea) {
        BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
        return ea.population().ind(i).attr();
    }
    
    //! Retrieve a const reference to an individual's attributes given a population const_iterator.
    template <typename EA>
    const typename EA::individual_attr_type& attr(typename EA::population_type::const_iterator i, EA& ea) {
        BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
        return ea.population().ind(i).attr();
    }
    
    //! Retrieve a reference to an individual's attributes given an individual pointer.
    template <typename EA>
    typename EA::individual_attr_type& attr(typename EA::individual_ptr_type p, EA& ea) {
        BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
        return p->attr();
    }
    
    template <typename EA>
    const typename EA::individual_ptr_type ptr(typename EA::population_type::const_iterator i, EA& ea) {
        BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
        return ea.population().ptr(i);
    }
    
    template <typename EA>
    typename EA::individual_ptr_type ptr(typename EA::population_entry_type i, EA& ea) {
        return ea.population().ptr(i);
    }
    
    template <typename EA>
    typename EA::individual_ptr_type ptr(typename EA::population_type::iterator i, EA& ea) {
        BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
        return ea.population().ptr(i);
    }

    template <typename EA>
    typename EA::individual_ptr_type make_individual_ptr(const typename EA::individual_type& i, EA& ea) {
        typename EA::individual_ptr_type p(new typename EA::individual_type(i));
        return p;
    }
    
    template <typename EA>
    typename EA::population_entry_type make_population_entry(const typename EA::individual_type& i, EA& ea) {
        typename EA::individual_ptr_type p(new typename EA::individual_type(i));
        return ea.population().make_population_entry(p,ea);
    }
    
    template <typename EA>
    typename EA::population_entry_type make_population_entry(const typename EA::representation_type& r, EA& ea) {
        typename EA::individual_ptr_type p(new typename EA::individual_type(r));
        return ea.population().make_population_entry(p,ea);
    }    
    	    
} // ea

#endif

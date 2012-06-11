/* recombination.h
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
#ifndef _EA_RECOMBINATION_H_
#define _EA_RECOMBINATION_H_

#include <algorithm>
#include <utility>
#include <ea/interface.h>

namespace ea {
    namespace recombination {
        
        /*! Asexual reproduction.
         */
        struct asexual {
            std::size_t capacity() const { return 1; }
            
            //! Asexual reproduction (copies a single parent).
            template <typename Population, typename EA>
            void operator()(Population& parents, Population& offspring, EA& ea) {
                offspring.append(make_population_entry(ind(parents.begin(),ea).repr(),ea));
            }
        };
        
        
        /*! Single-point crossover.
         */
        struct single_point_crossover {
            std::size_t capacity() const { return 2; }
            
            //! Perform single-point crossover on two parents to produce two offspring.
            template <typename Population, typename EA>
            void operator()(Population& parents, Population& offspring, EA& ea) {
                // build the offspring:
                typename Population::iterator p=parents.begin();
                typename EA::representation_type o1=representation(ind(p,ea),ea);
                typename EA::representation_type o2=representation(ind(++p,ea),ea);
                
                // they need to be the same size...
                assert(o1.size() == o2.size());
                
                // select the crossover point:
                std::size_t xover = ea.rng()(o1.size());
                
                // and swap [begin,xover) between o1 and o2:
                std::swap_ranges(o1.begin(), o1.begin()+xover, o2.begin());
                
                // output the individuals:
                offspring.append(make_population_entry(typename EA::individual_type(o1),ea));
                offspring.append(make_population_entry(typename EA::individual_type(o2),ea));
            }
        };
        
        
        /*! Two-point crossover.
         */
        struct two_point_crossover {
            std::size_t capacity() const { return 2; }

            //! Perform two-point crossover on two parents to produce two offspring.
            template <typename Population, typename EA>
            void operator()(Population& parents, Population& offspring, EA& ea) {
                // build the offspring:
                typename Population::iterator p=parents.begin();
                typename EA::representation_type o1=ind(p,ea).repr();
                typename EA::representation_type o2=ind(++p,ea).repr();
                
                // they need to be the same size...
                assert(o1.size() == o2.size());

                // select the crossover points:
                std::pair<std::size_t, std::size_t> xover = ea.rng().choose_two(static_cast<std::size_t>(0), o1.size());
                
                // and swap [begin+first,begin+second) between o1 and o2:
                std::swap_ranges(o1.begin()+xover.first, o1.begin()+xover.second, o2.begin()+xover.first);
                
                // output the individuals:
                offspring.append(make_population_entry(typename EA::individual_type(o1),ea));
                offspring.append(make_population_entry(typename EA::individual_type(o2),ea));
            }
        };
        
    } // recombination
} // ea

#endif

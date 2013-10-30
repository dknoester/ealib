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


namespace ealib {
    
    /*! Common inheritance details.
     */
    template <typename EA>
    void inherits_from(typename EA::individual_type& parent, typename EA::individual_type& offspring, EA& ea) {
        offspring.name() = next<INDIVIDUAL_COUNT>(ea);
        offspring.generation() = parent.generation() + 1.0;
        offspring.birth_update() = ea.current_update();
    }
    
    
    /*! Common inheritance details.
     */
    template <typename Population, typename EA>
    void inherits(Population& parents, Population& offspring, EA& ea) {
        for(typename Population::iterator i=offspring.begin(); i!=offspring.end(); ++i) {
            inherits_from(**parents.begin(), **i, ea);
            ea.events().inheritance(parents, **i, ea);
        }
    }
    
    
    /*! Recombine parents to generate offspring via the given recombination operator.
     */
    template <typename Population, typename Recombinator, typename EA>
    void recombine(Population& parents, Population& offspring, Recombinator rec, EA& ea) {
        rec(parents, offspring, ea);
        inherits(parents, offspring, ea);
    }
    
    
    /*! Recombine parents selected from the given population to generate n offspring.
     */
    template <typename Population, typename Selector, typename Recombinator, typename EA>
    void recombine_n(Population& population, Population& offspring, Selector sel, Recombinator rec, std::size_t n, EA& ea) {
        while(offspring.size() < n) {
            Population p, o; // parents, offspring
            sel(population, p, rec.capacity(), ea); // select parents
            rec(p, o, ea); // recombine parents to produce offspring
            inherits(p, o, ea);
            offspring.insert(offspring.end(), o.begin(), o.end());
        }
        offspring.resize(n); // in case extra were generated...
    }
    
    /*! Meta-data that governs how subpopulation recombination works.
     */
    LIBEA_MD_DECL(PROPAGULE_SIZE, "ea.recombination.propagule_size", std::size_t);
    
    namespace recombination {
        
        /*! Asexual reproduction.
         */
        struct asexual {
            std::size_t capacity() const { return 1; }
            
            //! Asexual reproduction (copies a single parent's representation).
            template <typename Population, typename EA>
            void operator()(Population& parents, Population& offspring, EA& ea) {
                offspring.insert(offspring.end(),
                                 ea.make_individual(parents.front()->repr()));
            }
        };
        
        /*! Asexual propagule creation for sub-populations. Individuals are sampled from the
         parent subpopulation without replacement.
         */
        struct propagule_without_replacement {
            std::size_t capacity() const { return 1; }
            
            //! Asexual reproduction (copies a single parent's representation).
            template <typename Population, typename EA>
            void operator()(Population& parents, Population& offspring, EA& ea) {
                
                std::size_t prop_size = get<PROPAGULE_SIZE>(ea,1);
                assert(prop_size > 0);
                if (prop_size > parents[0]->size()) {
                    prop_size = parents[0]->size();
                }
                
                // p is a subpopulation
                typename EA::individual_ptr_type p = ea.make_individual();
                
                // and fill up the subpopulation
                typedef typename Population::value_type::element_type::population_type propagule_type;
                propagule_type propagule;
                ea.rng().sample_without_replacement(parents[0]->population().begin(),
                                                    parents[0]->population().end(),
                                                    std::back_inserter(propagule),
                                                    prop_size);
                for(typename propagule_type::iterator i=propagule.begin(); i!=propagule.end(); ++i) {
                    // grab the original part of the propagule's genome; note that it could have been
                    // changed (implicit-like mutations):
                    typename EA::individual_type::representation_type r((*i)->repr().begin(),
                                                                        (*i)->repr().begin()+(*i)->hw().original_size());
                    typename EA::individual_type::individual_ptr_type q = p->make_individual(r);
                    
                    p->append(q);
                }
                
                offspring.insert(offspring.end(),p);
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
                offspring.insert(offspring.end(), ea.make_individual(o1));
                offspring.insert(offspring.end(), ea.make_individual(o2));
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
                typename EA::representation_type o1=(*p)->repr();
                typename EA::representation_type o2=(*++p)->repr();
                
                // they need to be the same size...
                assert(o1.size() == o2.size());
                
                // select the crossover points:
                std::pair<std::size_t, std::size_t> xover = ea.rng().choose_two(static_cast<std::size_t>(0), o1.size());
                
                // and swap [begin+first,begin+second) between o1 and o2:
                std::swap_ranges(o1.begin()+xover.first, o1.begin()+xover.second, o2.begin()+xover.first);
                
                // output the individuals:
                offspring.insert(offspring.end(), ea.make_individual(o1));
                offspring.insert(offspring.end(), ea.make_individual(o2));
            }
        };
        
    } // recombination
} // ea

#endif

/* nsga2.h
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
#ifndef _EA_NSGA2_H_
#define _EA_NSGA2_H_

#include <boost/serialization/nvp.hpp>
#include <algorithm>

#include <ea/individual.h>
#include <ea/metadata.h>
#include <ea/comparators.h>
#include <ea/access.h>
#include <ea/selection/proportionate.h>
#include <ea/selection/tournament.h>
#include <ea/traits.h>

namespace ealib {
    
    /*! NSGA2 evolutionary algorithm.
     
     Note: This is due for cleanup, just like QHFC.  For now, declare an NSGA2
     instance like this:
     typedef evolutionary_algorithm<
     bitstring,
     mutation::operators::per_site<mutation::site::bitflip>, // mutation operator
     multi_all_ones,
     configuration,
     recombination::two_point_crossover,
     generational_models::nsga2,
     nsga2_attributes
     > ea_type;
     */
    
    //! Attributes that must be added to individuals to support NSGA2.
    template <typename T>
    struct nsga2_traits : fitness_trait<T> {
        typedef fitness_trait<T> parent;
        typedef std::vector<typename T::individual_ptr_type> dominated_population_type;
        
        //! Constructor.
        nsga2_traits() : n(0), rank(0), distance(0.0) {
        }

        //! Serialization.
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("fitness_trait", boost::serialization::base_object<parent>(*this));
        }
        
        dominated_population_type S; //!< Population of individuals that are dominated by this individual.
        int n; //!< Number of individuals dominating this individual.
        int rank; //!< Rank of this individual.
        double distance; //<! Crowding distance.
    };
    
    
    /*! Crowding comparison operator, <_n.
     
     If a has lower rank than b, return true.
     If a has the same rank as b, but greater crowding distance, return true.
     Otherwise, return false.
     */
    template <typename AttributeAccessor, typename EA>
    struct crowding_comparator {
        //! Constructor.
        crowding_comparator(EA& ea) : _ea(ea) {
        }
        
        //! Returns true if a <_n b, false otherwise.
        bool operator()(typename EA::individual_ptr_type a, typename EA::individual_ptr_type b) {
            return (_acc(*a,_ea).rank < _acc(*b,_ea).rank)
            || ((_acc(*a,_ea).rank == _acc(*b,_ea).rank) && (_acc(*a,_ea).distance > _acc(*b,_ea).distance));
        }
        
        AttributeAccessor _acc;
        EA& _ea; //!< Reference to the EA in which the individuals to be compared reside.
    };
    
    namespace selection {
        
        /*! NSGA2 selection strategy.
         */
        struct nsga2 {
            
            //! Initializing constructor.
			template <typename Population, typename EA>
			nsga2(std::size_t n, Population& src, EA& ea) {
            }
            
            //! Returns true if a dominates b.
            template <typename Individual, typename EA>
            bool dominates(Individual& a, Individual& b, EA& ea) {
                const typename EA::fitness_type& fa=ealib::fitness(a,ea);
                const typename EA::fitness_type& fb=ealib::fitness(b,ea);
                assert(fa.size() == fb.size());
                
                bool any=false, all=true;
                
                for(std::size_t i=0; i<fa.size(); ++i) {
                    any = any || (fa[i] > fb[i]);
                    all = all && (fa[i] >= fb[i]);
                }
                return any && all;
            }
            
            //! Calculates crowding distance among individuals in population I.
            template <typename Population, typename EA>
            void crowding_distance(Population& I, EA& ea) {
                for(typename Population::iterator i=I.begin(); i!=I.end(); ++i) {
                    (*i)->traits().distance = 0.0;
                }
                
                std::size_t M = ealib::fitness(**I.begin(),ea).size(); // how many objectives?
                
                for(std::size_t m=0; m<M; ++m) {
                    std::sort(I.begin(), I.end(), comparators::objective<EA>(m,ea));
                    
                    (*I.begin())->traits().distance = std::numeric_limits<double>::max();
                    (*I.rbegin())->traits().distance = std::numeric_limits<double>::max();
                    
                    for(std::size_t i=1; i<(I.size()-1); ++i) {
                        I[i]->traits().distance += (ealib::fitness(*I[i+1],ea)[m] - ealib::fitness(*I[i-1],ea)[m]) / ea.fitness_function().range(m);
                        
//                        I[i]->traits().distance += (I[i+1]->fitness()[m] - I[i-1]->fitness()[m]) / ea.fitness_function().range(m);
                    }
                }
            }
            
            //! Sort at least n individuals from population P into fronts F.
            template <typename Population, typename PopulationMap, typename EA>
            void nondominated_sort(Population& P, std::size_t n, PopulationMap& F, EA& ea) {
                for(typename Population::iterator p=P.begin(); p!=P.end(); ++p) {
                    (*p)->traits().S.clear();
                    (*p)->traits().n = 0;
                    
                    for(typename Population::iterator q=P.begin(); q!=P.end(); ++q) {
                        if(p!=q) {
                            if(dominates(**p,**q,ea)) {
                                (*p)->traits().S.push_back(*q);
                            } else if(dominates(**q,**p,ea)) {
                                ++(*p)->traits().n;
                            }
                        }
                    }
                    if((*p)->traits().n == 0) {
                        (*p)->traits().rank = 0;
                        F[0].push_back(*p);
                    }
                }
                
                std::size_t i=0;
                while((!F[i].empty()) && (n>0)) {
                    Population Q;
                    for(typename Population::iterator p=F[i].begin(); p!=F[i].end(); ++p) {
                        for(typename Population::iterator q=(*p)->traits().S.begin(); q!=(*p)->traits().S.end(); ++q) {
                            --(*q)->traits().n;
                            if((*q)->traits().n == 0) {
                                (*q)->traits().rank = i+1;
                                Q.push_back(*q);
                            }
                        }
                    }
                    ++i;
                    n -= Q.size();
                    std::swap(F[i], Q);
                }
            }
            
			//! Select n individuals via tournament selection.
			template <typename Population, typename EA>
			void operator()(Population& src, Population& dst, std::size_t n, EA& ea) {
                // build up the fronts:
                std::map<int,Population> F;
                nondominated_sort(src, n, F, ea);
                
                // the set of all possible parents are pulled from the best fronts:
                for(std::size_t i=0; (i<F.size()) && (dst.size()<n); ++i) {
                    crowding_distance(F[i],ea);
                    dst.insert(dst.end(),
                               F[i].begin(),
                               F[i].begin() + std::min(F[i].size(), (n-dst.size())));
                }
            }
        };
    }
    
	namespace generational_models {
		
		/*! NSGA 2 generational model.
		 
         This generational model defines the NSGA 2 multiobjective evolutionary
         optimization algorithm EA~\cite{deb}.  It is comprised of three parts (below),
         and then the algorithm itself.
		 
         Fast non-dominated sort:
         P = population, F = fronts
         S_p = solutions dominated by p
         n_p = # of solutions dominating p
         F_i = ith front
         for each p in P
         S_p = 0, n_p = 0
         for each q in P (!=p)
         if p dom q
         S_p += q
         else
         ++n_p
         if n_p == 0
         p_rank = 1
         F_1 += p
         
         i=1
         while F_i not empty
         Q = empty
         for each p in F_i
         for each q in S_p
         --n_q
         if n_q == 0
         q_rank = i+1
         Q += q
         ++i
         F_i = Q
         
         Calculating crowding distance:
         I = a single nondominated set (ie, F_i)
         l = |I|
         I[1:l]_distance = 0
         for each objective m:
         I = sort(I,m) // sort I in ascending order by m
         I[1]_distance = I[l]_distance = inf // set the crowding distance of the endpoints to infinity (make sure they're included)
         for i=2:(l-1)
         I[i]_distance = I[i]_distance + (I[i+1].m - I[i-1].m)/(f^max_m - f^min_m)
         
         Crowding comparison operator, <_n:
         i_rank == rank (front index)
         i_distance == crowding distance
         i <_n j if((i_rank < j_rank) || ((i_rank == j_rank) && (i_distance > j_distance)))
         in words: prefer lower (better) rank, or if same, prefer less crowded
         
         NSGA2 algorithm:
         R_t = P_t union Q_t // combine parent and offspring populations, |R_t| == 2N
         F = fast_non_dominated_sort(R_t)
         P_t+1 = empty, i=1
         until |P_t+1| + |F_i| <= N // while next population is still less than N (pop size)
         crowding_distance_assignment(F_i) // calculate crowding distance
         P_t+1 = P_t+1 union F_i  // add the ith non-dominated front
         ++i
         sort(F_i) // sort the last front that we added with <_n
         P_t+1 = P_t+1 U F_i[1:(N-|P_t+1|)]
         Q_t+1 = make_new_pop(P_t+1) // selection, crossover, mutation
         binary tournament selection, based on <_n
         ++generation
		 */
		struct nsga2 {
            
            //! Apply NSGA2 to produce the next generation.
			template <typename Population, typename EA>
			void operator()(Population& population, EA& ea) {
                std::size_t n=get<POPULATION_SIZE>(ea)/2; // nsga2 uses a population split evenly between parents and offspring
                
                // select the parents via nondominated sorting:
                Population parents;
                select_n<selection::nsga2>(population, parents, n, ea);
                
                // select parents & recombine to create offspring:
                Population offspring;
                recombine_n(parents, offspring,
                            selection::tournament<access::traits,crowding_comparator>(n,parents,ea),
                            typename EA::recombination_operator_type(),
                            n, ea);
                
                // mutate the offspring:
				mutate(offspring.begin(), offspring.end(), ea);
                
                // add the offspring to the parent population to create the next generation:
                parents.insert(parents.end(), offspring.begin(), offspring.end());
                
                // and swap 'em in:
                std::swap(population, parents);
			}
		};
		
	} // generational_models
} // ea

#endif

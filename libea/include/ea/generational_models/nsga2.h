#ifndef _EA_GENERATIONAL_MODELS_NSGA2_H_
#define _EA_GENERATIONAL_MODELS_NSGA2_H_

#include <boost/serialization/nvp.hpp>
#include <algorithm>
#include <ea/interface.h>
#include <ea/individual.h>
#include <ea/meta_data.h>
#include <ea/generational_model.h>
#include <ea/comparators.h>
#include <ea/selection/proportional.h>
#include <ea/selection/tournament.h>

namespace ea {
    
    //! Attributes that must be added to individuals to support NSGA2.
    template <typename EA>
    struct nsga2_attrs {
        typedef typename EA::population_type population_type;
        
        //! Constructor.
        nsga2_attrs() : n(0), rank(0), distance(0.0) {
        }
        
        //! Serialize some of these attributes.
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & BOOST_SERIALIZATION_NVP(rank);
            ar & BOOST_SERIALIZATION_NVP(distance);
        }

        population_type S; //!< Population of individuals that are dominated by this individual.
        std::size_t n; //!< Number of individuals dominating this individual.
        std::size_t rank; //!< Rank of this individual.
        double distance; //<! Crowding distance.
    };
    
    
    /*! Crowding comparison operator, <_n.  
     
     If a has lower rank than b, return true.
     If a has the same rank as b, but greater crowding distance, return true.
     Otherwise, return false.
     */
    struct crowding_comparator {
        template <typename IndividualPtr>
        bool operator()(IndividualPtr a, IndividualPtr b) {
            return (a->attr().rank < b->attr().rank) || ((a->attr().rank == b->attr().rank) && (a->attr().distance > b->attr().distance));
        }
    };

    
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
		struct nsga2 : public generational_model {
            
            
            //! Calculates crowding distance among individuals in population I.
            template <typename Population, typename EA>
            void crowding_distance(Population& I, EA& ea) {
                for(typename Population::iterator i=I.begin(); i!=I.end(); ++i) {
                    (*i)->attr().distance = 0.0;
                }
                
                std::size_t M = (*I.begin())->fitness().size(); // how many objectives?
                
                for(std::size_t m=0; m<M; ++m) {
                    std::sort(I.begin(), I.end(), comparators::objective(m));
                    
                    (*I.begin())->attr().distance = std::numeric_limits<double>::max();
                    (*I.rbegin())->attr().distance = std::numeric_limits<double>::max();
                    
                    for(std::size_t i=1; i<(I.size()-1); ++i) {
                        I[i]->attr().distance += (I[i+1]->fitness()[m] - I[i-1]->fitness()[m]) / ea.fitness_function().range(m);
                    }
                }
            }
            
            //! Returns true if a dominates b.
            template <typename Individual>
            bool dominates(Individual& a, Individual& b) {
                const typename Individual::fitness_type& fa=a.fitness();
                const typename Individual::fitness_type& fb=b.fitness();
                assert(fa.size() == fb.size());
                
                bool any=false, all=true;
                
                for(std::size_t i=0; i<fa.size(); ++i) {
                    any |= (fa[i] > fb[i]);
                    all &= (fa[i] >= fb[i]);
                }
                return any && all;
            }

            //! Sort at least n individuals from population P into fronts F.
            template <typename Population, typename PopulationMap, typename EA>
            void nondominated_sort(Population& P, std::size_t n, PopulationMap& F, EA& ea) {
                for(typename Population::iterator p=P.begin(); p!=P.end(); ++p) {
                    attr(p,ea).S.clear();
                    attr(p,ea).n = 0;
                    
                    for(typename Population::iterator q=P.begin(); q!=P.end(); ++q) {
                        if(p!=q) {
                            if(dominates(**p,**q)) {
                                attr(p,ea).S.append(q);
                            } else if(dominates(**q,**p)) {
                                ++attr(p,ea).n;
                            }
                        }
                    }
                    if(attr(p,ea).n == 0) {
                        attr(p,ea).rank = 0;
                        F[0].append(p);
                    }
                }
                
                std::size_t i=0;
                while((!F[i].empty()) && (n>0)) {
                    Population Q;
                    for(typename Population::iterator p=F[i].begin(); p!=F[i].end(); ++p) {
                        for(typename Population::iterator q=attr(p,ea).S.begin(); q!=attr(p,ea).S.end(); ++q) {
                            --attr(q,ea).n;
                            if(attr(q,ea).n == 0) {
                                attr(q,ea).rank = i+1;
                                Q.append(q);
                            }
                        }
                    }
                    ++i;
                    n -= Q.size();
                    std::swap(F[i], Q);
                }
            }
            
                     
			//! Apply NSGA2 to produce the next generation.
			template <typename Population, typename EA>
			void operator()(Population& population, EA& ea) {
				BOOST_CONCEPT_ASSERT((PopulationConcept<Population>));
				BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
                
                std::size_t N=get<POPULATION_SIZE>(ea)/2; // nsga2 uses a population split evenly between parents and offspring
                
                // build up the fronts:
                std::map<int,Population> F;
                nondominated_sort(population, N, F, ea);
                
                // the set of all possible parents are pulled from the best fronts:
                Population parents;
                for(std::size_t i=0; (i<F.size()) && (parents.size()<N); ++i) {
                    crowding_distance(F[i],ea);
                    parents.append(F[i].begin(),
                                   F[i].begin() + std::min(F[i].size(), (N-parents.size())));
                }

                // select parents & recombine to create offspring:
                Population offspring;                
                recombine_n(parents, offspring,
                            selection::tournament<crowding_comparator>(N,parents,ea),
                            typename EA::recombination_operator_type(),
                            N, ea);

                // mutate the offspring:
				mutate(offspring.begin(), offspring.end(), ea);

                // add the offspring to the parent population to create the next generation:
                parents.append(offspring.begin(), offspring.end());

                // calculate fitness:
                calculate_fitness(parents.begin(), parents.end(), ea);
                
                // and swap 'em in:
                std::swap(population, parents);
			}
		};
		
	} // generational_models
} // ea

#endif

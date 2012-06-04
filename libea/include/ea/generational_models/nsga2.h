#ifndef _EA_GENERATIONAL_MODELS_NSGA2_H_
#define _EA_GENERATIONAL_MODELS_NSGA2_H_

#include <algorithm>
#include <ea/interface.h>
#include <ea/individual.h>
#include <ea/meta_data.h>
#include <ea/generational_model.h>
#include <ea/selection/fitness_proportional.h>
#include <ea/selection/tournament.h>

namespace ea {
    
    template <typename Representation, typename FitnessFunction>
	class nsga2_individual : public individual<Representation, FitnessFunction> {
    public:
        typedef Representation representation_type;
		typedef FitnessFunction fitness_function_type;
		typedef typename fitness_function_type::value_type fitness_type;
        typedef individual<representation_type,fitness_function_type> base_type;
        typedef nsga2_individual<representation_type,fitness_function_type> individual_type;
        typedef boost::shared_ptr<individual_type> individual_ptr_type;
        typedef population<individual_type,individual_ptr_type> population_type;
        
        //! Constructor.
        nsga2_individual() : base_type(), n(0), rank(0), distance(0.0) {
        }
        
        //! Copy constructor.
        nsga2_individual(const nsga2_individual& that) : base_type(that), n(0), rank(0), distance(0.0) {
        }
        
        //! Constructor that builds an individual from a representation.
		nsga2_individual(const representation_type& r) : base_type(r), n(0), rank(0), distance(0.0) {
		}
        
        //! Assignment operator.
        nsga2_individual& operator=(const nsga2_individual& that) {
            if(this != & that) {
                base_type::operator=(that);
                n = 0;
                rank = 0;
                distance = 0.0;
                S.clear();
            }
            return *this;
        }
        
        //! Destructor.
        virtual ~nsga2_individual() {
        }
        
        population_type S;
        std::size_t n;
        std::size_t rank;
        double distance;
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

            //! Comparator for sorting a population by the m'th objective.
            struct objective_comparator {
                objective_comparator(std::size_t m) : _m(m) {
                }
                
                template <typename IndividualPtr>
                bool operator()(IndividualPtr& a, IndividualPtr& b) {
                    return a->fitness()[_m] < b->fitness()[_m];
                }
                
                std::size_t _m;
            };
            
            //! Calculates crowding distance among individuals in population I.
            template <typename Population>
            void crowding_distance(Population& I) {
                for(typename Population::iterator i=I.begin(); i!=I.end(); ++i) {
                    (*i)->distance = 0.0;
                }
                
                std::size_t M = (*I.begin())->fitness().size(); // how many objectives?
                
                for(std::size_t m=0; m<M; ++m) {
                    std::sort(I.begin(), I.end(), objective_comparator(m));
                    double range = (*I.rbegin())->fitness()[m] - (*I.begin())->fitness()[m];
                    if(range == 0.0) {
                        range = 1.0;
                    }
                    
                    (*I.begin())->distance = std::numeric_limits<double>::max();
                    (*I.rbegin())->distance = std::numeric_limits<double>::max();
                    
                    for(std::size_t i=1; i<(I.size()-1); ++i) {
                        I[i]->distance += (I[i+1]->fitness()[m] - I[i-1]->fitness()[m]) / range;
                    }
                }
            }
            
            /*! Crowding comparison operator, <_n.  
             
             If a has lower rank than b, return true.
             If a has the same rank as b, but greater crowding distance, return true.
             Otherwise, return false.
             */
            struct crowding_comparator {
                template <typename IndividualPtr>
                bool operator()(IndividualPtr a, IndividualPtr b) {
                    return (a->rank < b->rank) || ((a->rank == b->rank) && (a->distance > b->distance));
                }
            };
            
            //! Crowding-based tournament selection.
            struct crowding_tournament {
                template <typename Population, typename EA>
                crowding_tournament(std::size_t n, Population& src, EA& ea) { 
                }
                
                template <typename Population, typename EA>
                void operator()(Population& src, Population& dst, std::size_t n, EA& ea) {
                    std::size_t N = get<TOURNAMENT_SELECTION_N>(ea);
                    std::size_t K = get<TOURNAMENT_SELECTION_K>(ea);
                    while(n > 0) {
                        Population tourney;
                        ea.rng().sample_without_replacement(src.begin(), src.end(), std::back_inserter(tourney), N);
                        
                        std::sort(tourney.begin(), tourney.end(), crowding_comparator());
                        typename Population::reverse_iterator rl=tourney.rbegin();
                        std::size_t copy_size = std::min(n,K);
                        std::advance(rl, copy_size);
                        dst.append(tourney.rbegin(), rl);
                        n -= copy_size;
                    }
                }
            };
            
            //! Returns true if a dominates b (all fitness values in a >= b).
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
            template <typename Population, typename PopulationMap>
            void nondominated_sort(Population& P, std::size_t n, PopulationMap& F) {
                for(typename Population::iterator p=P.begin(); p!=P.end(); ++p) {
                    (*p)->S.clear();
                    (*p)->n = 0;                    
                    for(typename Population::iterator q=P.begin(); q!=P.end(); ++q) {
                        if(p!=q) {
                            if(dominates(**p,**q)) {
                                (*p)->S.append(q);
                            } else if(dominates(**q,**p)) {
                                ++(*p)->n;
                            }
                        }
                    }
                    if((*p)->n == 0) {
                        (*p)->rank = 0;
                        F[0].append(p);
                    }
                }
                
                std::size_t i=0;
                while((!F[i].empty()) && (n>0)) {
                    Population Q;
                    for(typename Population::iterator p=F[i].begin(); p!=F[i].end(); ++p) {
                        for(typename Population::iterator q=(*p)->S.begin(); q!=(*p)->S.end(); ++q) {
                            --(*q)->n;
                            if((*q)->n == 0) {
                                (*q)->rank = i+1;
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
                nondominated_sort(population, N, F);
                
                // the set of all possible parents are pulled from the best fronts:
                Population parents;
                for(std::size_t i=0; (i<F.size()) && (parents.size()<N); ++i) {
                    crowding_distance(F[i]);
                    
                    parents.append(F[i].begin(), 
                                   F[i].begin() + std::min(F[i].size(), (N-parents.size())));
                }

                // select parents & recombine to create offspring:
                Population offspring;                
                recombine_n(parents, offspring,
                            crowding_tournament(N,parents,ea),
                            typename EA::recombination_operator_type(),
                            N, ea);

                // mutate the offspring:
				mutate(offspring.begin(), offspring.end(), ea);

                // calculate their fitness:
                calculate_fitness(offspring.begin(), offspring.end(), ea);
                
                // add the offspring to the parent population to create the next generation:
                parents.append(offspring.begin(), offspring.end());

                // and swap 'em in:
                std::swap(population, parents);
			}
		};
		
	} // generational_models
} // ea

#endif

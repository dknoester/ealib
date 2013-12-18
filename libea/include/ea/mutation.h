/* mutation.h
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
#ifndef _EA_MUTATION_H_
#define _EA_MUTATION_H_

#include <ea/algorithm.h>
#include <ea/concepts.h>
#include <ea/meta_data.h>


namespace ealib {
    
    // ea.mutation.*
	LIBEA_MD_DECL(MUTATION_GENOMIC_P, "ea.mutation.genomic.p", double);
	LIBEA_MD_DECL(MUTATION_PER_SITE_P, "ea.mutation.site.p", double);
	LIBEA_MD_DECL(MUTATION_DUPLICATION_P, "ea.mutation.duplication.p", double);

	LIBEA_MD_DECL(MUTATION_DELETION_P, "ea.mutation.deletion.p", double);
	LIBEA_MD_DECL(MUTATION_INSERTION_P, "ea.mutation.insertion.p", double);
    LIBEA_MD_DECL(MUTATION_INDEL_MAX_SIZE, "ea.mutation.indel.max_size", int);
    LIBEA_MD_DECL(MUTATION_INDEL_MIN_SIZE, "ea.mutation.indel.min_size", int);
    
    LIBEA_MD_DECL(MUTATION_UNIFORM_INT_MIN, "ea.mutation.uniform_integer.min", int);
    LIBEA_MD_DECL(MUTATION_UNIFORM_INT_MAX, "ea.mutation.uniform_integer.max", int);
    LIBEA_MD_DECL(MUTATION_UNIFORM_REAL_MIN, "ea.mutation.uniform_real.min", double);
    LIBEA_MD_DECL(MUTATION_UNIFORM_REAL_MAX, "ea.mutation.uniform_real.max", double);
    LIBEA_MD_DECL(MUTATION_NORMAL_REAL_MEAN, "ea.mutation.normal_real.mean", double);
    LIBEA_MD_DECL(MUTATION_NORMAL_REAL_VAR, "ea.mutation.normal_real.var", double);
    LIBEA_MD_DECL(MUTATION_CLIP_MIN, "ea.mutation.clip.min", double);
    LIBEA_MD_DECL(MUTATION_CLIP_MAX, "ea.mutation.clip.max", double);
    LIBEA_MD_DECL(MUTATION_ZERO_P, "ea.mutation.zero.p", double);

    LIBEA_MD_DECL(MUTATION_SUBPOP_DELETION_P, "ea.mutation.subpopulation.deletion.p", double);
    LIBEA_MD_DECL(MUTATION_SUBPOP_INSERTION_P, "ea.mutation.subpopulation.insertion.p", double);
    LIBEA_MD_DECL(MUTATION_SUBPOP_MIN_SIZE, "ea.mutation.subpopulation.min_size", int);
    LIBEA_MD_DECL(MUTATION_SUBPOP_MAX_SIZE, "ea.mutation.subpopulation.max_size", int);

	/*! Unconditionally mutate an individual.
	 */
	template <typename Mutator, typename EA>
	void mutate(typename EA::individual_type& ind, Mutator& mutator, EA& ea) {
		BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
		BOOST_CONCEPT_ASSERT((MutationOperatorConcept<Mutator,EA>));
		mutator(ind, ea);
	}
	
	
	/*! Unconditionally mutate an individual using the EA's embedded types.
	 */
	template <typename EA>
	void mutate(typename EA::individual_type& ind, EA& ea) {
		typename EA::mutation_operator_type mutator;
		mutate(ind, mutator, ea);
	}
    
	
	/*! Unconditionally mutate a range of individuals.
	 */
	template <typename ForwardIterator, typename Mutator, typename EA>
	void mutate(ForwardIterator first, ForwardIterator last, Mutator& mutator, EA& ea) {
		BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
		BOOST_CONCEPT_ASSERT((MutationOperatorConcept<Mutator,EA>));
		for( ; first!=last; ++first) {
			mutate(**first, mutator, ea);
		}
	}
    
	
	/*! Unconditionally mutate a range of individuals using the EA's embedded types.
	 */
	template <typename ForwardIterator, typename EA>
	void mutate(ForwardIterator first, ForwardIterator last, EA& ea) {
		BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
		typename EA::mutation_operator_type mutator;
		mutate(first, last, mutator, ea);
	}
	
	
	/*! Probabilistically mutate a range of individuals.
	 */
	template <typename ForwardIterator, typename Mutator, typename EA>
	void mutate_p(ForwardIterator first, ForwardIterator last, Mutator& mutator, const double prob, EA& ea) {
		BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
		BOOST_CONCEPT_ASSERT((MutationOperatorConcept<Mutator,EA>));
		for( ; first!=last; ++first) {
			if(ea.rng().p(prob)) {
				mutate(ind(first,ea), mutator, ea);
			}
		}
	}
	
	
	/*! Probabilisitically mutate a range of individuals using the EA's embedded types.
	 */
	template <typename ForwardIterator, typename EA>
	void mutate_p(ForwardIterator first, ForwardIterator last, const double prob, EA& ea) {
		BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
		typename EA::mutation_operator_type mutator;
		mutate_p(first, last, mutator, prob, ea);
	}
    
    namespace mutation {
        
        namespace site {
            struct uniform_integer {
                template <typename Iterator, typename EA>
                void operator()(Iterator i, EA& ea) {
                    *i = ea.rng().uniform_integer(get<MUTATION_UNIFORM_INT_MIN>(ea), get<MUTATION_UNIFORM_INT_MAX>(ea));
                }
            };
            
            struct uniform_real {
                template <typename Iterator, typename EA>
                void operator()(Iterator i, EA& ea) {
                    *i = ea.rng().uniform_real(get<MUTATION_UNIFORM_REAL_MIN>(ea), get<MUTATION_UNIFORM_REAL_MAX>(ea));
                }
            };
            
            struct normal_real {
                template <typename Iterator, typename EA>
                void operator()(Iterator i, EA& ea) {
                    *i = ea.rng().normal_real(get<MUTATION_NORMAL_REAL_MEAN>(ea), get<MUTATION_NORMAL_REAL_VAR>(ea));
                }
            };
            
            struct relative_normal_real {
                template <typename Iterator, typename EA>
                void operator()(Iterator i, EA& ea) {
                    *i = ea.rng().normal_real(*i, get<MUTATION_NORMAL_REAL_VAR>(ea));
                }
            };
            
            struct bit {
                template <typename Iterator, typename EA>
                void operator()(Iterator i, EA& ea) {
                    *i = ea.rng().bit();
                }
            };
            
            struct bitflip {
                template <typename Iterator, typename EA>
                void operator()(Iterator i, EA& ea) {
                    (*i) ^= 0x01;
                }
            };
            
            template <typename MutationType>
            struct clip {
                typedef MutationType mutation_type;
                
                template <typename Iterator, typename EA>
                void operator()(Iterator i, EA& ea) {
                    _mt(i, ea);
                    *i = algorithm::clip(*i, get<MUTATION_CLIP_MIN>(ea), get<MUTATION_CLIP_MAX>(ea));
                }
                
                mutation_type _mt;
            };
            
            template <typename MutationType>
            struct zero {
                typedef MutationType mutation_type;
                
                template <typename Iterator, typename EA>
                void operator()(Iterator i, EA& ea) {
                    if(ea.rng().p(get<MUTATION_ZERO_P>(ea))) {
                        *i = 0.0;
                    } else {
                        _mt(i, ea);
                    }
                }
                
                mutation_type _mt;
            };
            
        } // site
        
        namespace operators {
            
            /*! Null mutation.
             */
            struct null_mutation {
                template <typename EA>
                void operator()(typename EA::individual_type& ind, EA& ea) {
                }
            };
            
            
            /*! Subpopulation mutation.
             
             Here, the EA is assumed to be a meta-population, individuals are in
             fact a subpopulation EA, and mutation involves operations on that EA.
             */
            struct subpopulation {
                template <typename EA>
                void operator()(typename EA::individual_type& ind, EA& ea) {
                    // first, see if we're going to alter the size of this EA:
                    if(ind.size() < static_cast<std::size_t>(get<MUTATION_SUBPOP_MAX_SIZE>(ea))
                       && ea.rng().p(get<MUTATION_SUBPOP_INSERTION_P>(ea))) {
                        put<POPULATION_SIZE>(get<POPULATION_SIZE>(ind)+1, ind);
                    }

                    if(ind.size() > static_cast<std::size_t>(get<MUTATION_SUBPOP_MIN_SIZE>(ea))
                       && ea.rng().p(get<MUTATION_SUBPOP_DELETION_P>(ea))) {
                        put<POPULATION_SIZE>(get<POPULATION_SIZE>(ind)-1, ind);
                    }

                    // and now update the subpopulation (this calls the subpopulation
                    // EA's generational model):
                    ind.update();
                }
            };

            /*! Single-point mutation.
             */
            template <typename MutationType>
            struct single_point {
                typedef MutationType mutation_type;
                
                //! Mutate a single point in the given representation.
                template <typename EA>
                void operator()(typename EA::individual_type& ind, EA& ea) {
                    typename EA::representation_type& repr=ind.repr();
                    _mt(ea.rng().choice(repr.begin(), repr.end()), ea);
                }
                
                mutation_type _mt;
            };
            
            
            /*! Per-site mutation.
             */
            template <typename MutationType>
            struct per_site {
                typedef MutationType mutation_type;
                
                //! Iterate through all elements in the given representation, possibly mutating them.
                template <typename EA>
                void operator()(typename EA::individual_type& ind, EA& ea) {
                    typename EA::representation_type& repr=ind.repr();
                    const double per_site_p=get<MUTATION_PER_SITE_P>(ea);
                    for(typename EA::representation_type::iterator i=repr.begin(); i!=repr.end(); ++i){
                        if(ea.rng().p(per_site_p)) {
                            _mt(i, ea);
                        }
                    }
                }
                
                mutation_type _mt;
            };
            
            
            /*! Insertion/deletion mutation operator.
             
             Inserts a random-sized copy of an existing portion of the genome, or
             deletes a random part of the genome.
             */
            template <typename MutationOperator>
            struct indel {
                typedef MutationOperator mutation_operator_type;
                
                //! Probabilistically perform indel mutations.
                template <typename EA>
                void operator()(typename EA::individual_type& ind, EA& ea) {
                    typename EA::representation_type& repr=ind.repr();
                    
                    // insertion:
                    if((repr.size() < static_cast<std::size_t>(get<REPRESENTATION_MAX_SIZE>(ea))) && ea.rng().p(get<MUTATION_INSERTION_P>(ea))) {
                        std::size_t csize = ea.rng()(get<MUTATION_INDEL_MIN_SIZE>(ea), get<MUTATION_INDEL_MAX_SIZE>(ea));
                        typename EA::representation_type::iterator src=ea.rng().choice(repr.begin(),
                                                                                       repr.begin() + (repr.size()-csize));
                        // copy to avoid undefined behavior
                        typename EA::representation_type chunk(src, src+csize);
                        repr.insert(ea.rng().choice(repr.begin(),repr.end()), chunk.begin(), chunk.end());
                    }
                    
                    // deletion:
                    if((repr.size() > static_cast<std::size_t>(get<REPRESENTATION_MIN_SIZE>(ea))) && ea.rng().p(get<MUTATION_DELETION_P>(ea))) {
                        std::size_t csize = ea.rng()(get<MUTATION_INDEL_MIN_SIZE>(ea), get<MUTATION_INDEL_MAX_SIZE>(ea));
                        typename EA::representation_type::iterator src=ea.rng().choice(repr.begin(),
                                                                                       repr.begin() + (repr.size()-csize));
                        repr.erase(src, src+csize);
                    }
                    
                    // then carry on with normal mutations:
                    _mt(ind, ea);
                }
                
                mutation_operator_type _mt;            
            };
            
        } // operators
    } // mutation
} // ea

#endif

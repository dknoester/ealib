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
#include <ea/meta_data.h>


namespace ea {
    
	/*! Unconditionally mutate an individual.
	 */	
	template <typename Mutator, typename EA>
	void mutate(typename EA::individual_type& ind, Mutator& mutator, EA& ea) {
		BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
		BOOST_CONCEPT_ASSERT((MutationOperatorConcept<Mutator,EA>));	
		mutator(ind.repr(), ea);
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
        
        struct uniform_integer {
            template <typename Representation, typename EA>
            void operator()(Representation& repr, typename Representation::iterator i, EA& ea) {
                *i = ea.rng().uniform_integer(get<MUTATION_UNIFORM_INT_MIN>(ea), get<MUTATION_UNIFORM_INT_MAX>(ea));
            }
        };
        
        struct uniform_real {
            template <typename Representation, typename EA>
            void operator()(Representation& repr, typename Representation::iterator i, EA& ea) {
                *i = ea.rng().uniform_real(get<MUTATION_UNIFORM_REAL_MIN>(ea), get<MUTATION_UNIFORM_REAL_MAX>(ea));
            }
        };
        
        struct normal_real {
            template <typename Representation, typename EA>
            void operator()(Representation& repr, typename Representation::iterator i, EA& ea) {
                *i = ea.rng().normal_real(get<MUTATION_NORMAL_REAL_MEAN>(ea), get<MUTATION_NORMAL_REAL_VAR>(ea));
            }
        };
        
        struct relative_normal_real {
            template <typename Representation, typename EA>
            void operator()(Representation& repr, typename Representation::iterator i, EA& ea) {
                *i = ea.rng().normal_real(*i, get<MUTATION_NORMAL_REAL_VAR>(ea));
            }
        };
        
        struct bit {
            template <typename Representation, typename EA>
            void operator()(Representation& repr, typename Representation::iterator i, EA& ea) {
                *i = ea.rng().bit();
            }
        };

        struct bitflip {
            template <typename Representation, typename EA>
            void operator()(Representation& repr, typename Representation::iterator i, EA& ea) {
                (*i) ^= 0x01;
            }
        };

        template <typename MutationType>
        struct clip {
            typedef MutationType mutation_type;
            
            template <typename Representation, typename EA>
            void operator()(Representation& repr, typename Representation::iterator i, EA& ea) {
                _mt(repr, i, ea);
                *i = algorithm::clip(*i, get<MUTATION_CLIP_MIN>(ea), get<MUTATION_CLIP_MAX>(ea));
            }
            
            mutation_type _mt;
        };

        template <typename MutationType>
        struct zero {
            typedef MutationType mutation_type;
            
            template <typename Representation, typename EA>
            void operator()(Representation& repr, typename Representation::iterator i, EA& ea) {
                if(ea.rng().p(get<MUTATION_ZERO_P>(ea))) {
                    *i = 0.0;
                } else {
                    _mt(repr, i, ea);
                }
            }
            
            mutation_type _mt;
        };

        /*! Single-point mutation.
         */
        template <typename MutationType>
        struct single_point {
            typedef MutationType mutation_type;
            
            //! Mutate a single point in the given representation.
            template <typename Representation, typename EA>
            void operator()(Representation& repr, EA& ea) {
                _mt(repr, ea.rng().choice(repr.begin(), repr.end()), ea);
            }
            
            mutation_type _mt;
        };
        
        
        /*! Per-site mutation.
         */
        template <typename MutationType>
        struct per_site {            
            typedef MutationType mutation_type;
            
            //! Iterate through all elements in the given representation, possibly mutating them.
            template <typename Representation, typename EA>
            void operator()(Representation& repr, EA& ea) {
                const double per_site_p=get<MUTATION_PER_SITE_P>(ea);
                for(typename Representation::iterator i=repr.begin(); i!=repr.end(); ++i){
                    if(ea.rng().p(per_site_p)) {
                        _mt(repr, i, ea);
                    }
                }
            }
            
            mutation_type _mt;
        };

        
        /*! Insertion/deletion (of fixed-size chunks) mutation type.
         
         \todo for variable size chunks, implement slip mutations.
         */
        template <typename MutationOperator>
        struct indel {
            typedef MutationOperator mutation_operator_type;

            //! Probabilistically perform indel mutations.
            template <typename Representation, typename EA>
            void operator()(Representation& repr, EA& ea) {
                // insertion:
                if((repr.size() < get<REPRESENTATION_MAX_SIZE>(ea)) && ea.rng().p(get<MUTATION_INSERTION_P>(ea))) {
                    typename Representation::iterator src=ea.rng().choice(repr.begin(), repr.begin()+(repr.size()-get<MUTATION_INDEL_CHUNK_SIZE>(ea)));
                    Representation chunk(src, src+get<MUTATION_INDEL_CHUNK_SIZE>(ea)); // copy to avoid undefined behavior
                    repr.insert(ea.rng().choice(repr.begin(),repr.end()), chunk.begin(), chunk.end());
                }
                
                // deletion:
                if((repr.size() > get<REPRESENTATION_MIN_SIZE>(ea)) && ea.rng().p(get<MUTATION_DELETION_P>(ea))) {
                    typename Representation::iterator src=ea.rng().choice(repr.begin(), repr.begin()+(repr.size()-get<MUTATION_INDEL_CHUNK_SIZE>(ea)));
                    repr.erase(src, src+get<MUTATION_INDEL_CHUNK_SIZE>(ea));
                }
                
                // then carry on with normal mutations:
                _mt(repr, ea);
            }
            
            mutation_operator_type _mt;            
        };
        
    } // mutation
} // ea

#endif

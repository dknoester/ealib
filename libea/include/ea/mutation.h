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
#include <ea/interface.h>
#include <ea/meta_data.h>


namespace ea {
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
        

        /*! Per-site mutation, with single insertion and deletion.
         */
        template <typename MutationType>
        struct per_site_indel {            
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
                
                if(ea.rng().p(get<MUTATION_INSERTION_P>(ea))) {
                    _mt(repr, repr.insert(ea.rng().choice(repr.begin(),repr.end()), typename Representation::codon_type()), ea);
                }
                if(ea.rng().p(get<MUTATION_DELETION_P>(ea))) {
                    repr.erase(ea.rng().choice(repr.begin(),repr.end()));
                }                
            }
            
            mutation_type _mt;
        };
        
    } // mutation
} // ea

#endif

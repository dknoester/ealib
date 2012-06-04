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
        
    } // mutation
} // ea

#endif

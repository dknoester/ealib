#ifndef _EA_COMPARATORS_H_
#define _EA_COMPARATORS_H_


namespace ea {
    namespace comparators {

        //! Compare individual pointers based on the natural order of their fitnesses.
        struct fitness {
            template <typename IndividualPtr>
            bool operator()(IndividualPtr x, IndividualPtr y) {
                return x->fitness() < y->fitness();
            }
        };
        
        /*! Compare individuals based on the natural order of their m'th objective,
         which is useful with multivalued fitnesses.
         */
        struct objective {
            objective(std::size_t m) : _m(m) {
            }
            
            template <typename IndividualPtr>
            bool operator()(IndividualPtr& a, IndividualPtr& b) {
                return a->fitness()[_m] < b->fitness()[_m];
            }
            
            std::size_t _m;
        };
        
    } // comparators
} // ea

#endif

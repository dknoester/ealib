#ifndef _TUTORIAL_6_H_
#define _TUTORIAL_6_H_

/* Bring in our previous work...
 */
#include "tutorial_5.h"

/* Ok, now let's address the proliferation of random_selection_* methods.
 
 We'll do this by making a generic selection method:
 */
template <typename Mutator>
void generic_random_selection(population_type& p, Mutator m, double pmutate=0.05) {
    random_shuffle(p.begin(), p.end());
    population_type offspring(p.begin(), p.begin()+static_cast<std::size_t>(0.05*population_size));
    
    for(population_type::iterator i=offspring.begin(); i!=offspring.end(); ++i) {
        if(pmutate < (static_cast<double>(rand())/RAND_MAX)) {
            generic_mutate(*i, m);
        }
    }
    
    p.insert(p.end(), offspring.begin(), offspring.end());
    random_shuffle(p.begin(), p.end());
    p.erase(p.begin()+population_size, p.end());
}

/* Now let's try it out...
 */

#endif

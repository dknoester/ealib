#ifndef _TUTORIAL_4_H_
#define _TUTORIAL_4_H_

/* Bring in our previous work...
 */
#include "tutorial_3.h"

/* Ok, one of the problems with the EA in part 3 is that the mutation function
 (aka, the "mutation operator") isn't very smart.  In particular, this line:

     *i = rand() % 2;

 means that when a site is mutated, it's new value is randomly selected from {0,1}.
 While this makes sense, there's a 50% chance that a mutation will result in no
 change.  So, let's see what we have to do to fix this problem.
 
 We'll start by defining a new mutation operator, which we'll call "smart_mutate":
 */
void smart_mutate(repr_type& r, double p=0.01) {
    for(repr_type::iterator i=r.begin(); i!=r.end(); ++i) {
        // note: this is not a good way to generate random numbers!  used here
        // for illustrative purposes only.
        if(p < (static_cast<double>(rand())/RAND_MAX)) {
            *i = !*i;
        }
    }
}

/* Ok.  Now, it was our random_selection_with_mutation function that actually
 called the mutation operator.  So, we have to go and edit that now, too:
 */
void random_selection_with_smart_mutation(population_type& p, double pmutate=0.05) {
    random_shuffle(p.begin(), p.end());
    population_type offspring(p.begin(), p.begin()+static_cast<std::size_t>(0.05*population_size));
    
    // mutate:
    for(population_type::iterator i=offspring.begin(); i!=offspring.end(); ++i) {
        if(pmutate < (static_cast<double>(rand())/RAND_MAX)) {
            smart_mutate(*i);
        }
    }
    
    p.insert(p.end(), offspring.begin(), offspring.end());
    random_shuffle(p.begin(), p.end());
    p.erase(p.begin()+population_size, p.end());
}

/* Let's see if this helped...
 */

#endif

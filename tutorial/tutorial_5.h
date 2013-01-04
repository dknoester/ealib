#ifndef _TUTORIAL_5_H_
#define _TUTORIAL_5_H_

/* Bring in our previous work...
 */
#include "tutorial_4.h"

/* Ok, one of the problems with the EA in part 4 is that the guts of the mutation
 operator and the random_selection_... functions are substantially the same.  Indeed,
 they each differ in a single line from their original version.
 
 These functions are small, so it's not really a big deal.  But... let's say
 we needed to make a change that touched all 4 of these pieces of code.
 
 For example, what if we decided to fix the random number generator by replacing
 it with a better one?  We'd have to go and fix these methods in double the number
 of places.  As the number of mutation operators increase, and the number of selection
 methods increase, this problem only gets worse.  The odds that something's missed
 steadily increase.
 
 So, what to do?  Well, generic programming (templating) can help.
 
 We'll start by defining a generic mutation operator:
 */
template <typename Mutator>
void generic_mutate(repr_type& r, Mutator m, double p=0.01) {
    for(repr_type::iterator i=r.begin(); i!=r.end(); ++i) {
        if(p < (static_cast<double>(rand())/RAND_MAX)) {
            *i = m(*i);
        }
    }
}

/* Note that the new value of a site results from a function call to "m", which
 is something we've called a Mutator.  Let's see what that looks like now:
 */
int flip(const int t) {
    return !t;
}

/* Well now, this is pretty simple.  We can also provide the original mutator:
 */
int random_bit(const int t) {
    return rand() % 2;
}

/* Ok, interesting.  This lets us rewrite our selection method:
 */
void random_selection_with_generic_mutation(population_type& p, double pmutate=0.05) {
    random_shuffle(p.begin(), p.end());
    population_type offspring(p.begin(), p.begin()+static_cast<std::size_t>(0.05*population_size));
    
    // mutate:
    for(population_type::iterator i=offspring.begin(); i!=offspring.end(); ++i) {
        if(pmutate < (static_cast<double>(rand())/RAND_MAX)) {
            generic_mutate(*i, flip);
        }
    }
    
    p.insert(p.end(), offspring.begin(), offspring.end());
    random_shuffle(p.begin(), p.end());
    p.erase(p.begin()+population_size, p.end());
}

/* Let's see if this helped...
 */

#endif

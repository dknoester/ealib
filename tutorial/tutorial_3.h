#ifndef _TUTORIAL_3_H_
#define _TUTORIAL_3_H_

/* Bring in our previous work:
 */
#include "tutorial_2.h"


/* We now need to add mutation and preferential survival.
 
 Let's first define a function that mutates a representation, with a configurable
 per-site mutation rate that defaults to 1%:
 */
void mutate(repr_type& r, double p=0.01) {
    for(repr_type::iterator i=r.begin(); i!=r.end(); ++i) {
        // note: this is not a good way to generate random numbers!  used here
        // for illustrative purposes only.
        if(p < (static_cast<double>(rand())/RAND_MAX)) {
            *i = rand() % 2;
        }
    }
}

/* Now for preferential survival, aka competition.
 
 We need to take highly-fit representations, and replicate them.  So, we'll simply
 assign each representation a probability of replicating that is derived from
 its fitness.
 */
void compete(population_type& p) {
    population_type next;
    
    std::size_t i=0;
    while(next.size() < p.size()) {
        // again, don't use this method for random numbers.
        if(all_ones(p[i]) >= (rand() % repr_size)) {
            next.push_back(p[i]);
        }
        
        i = ++i % p.size();
    }
    
    std::swap(p, next);
}

/* We also need to modify the random_selection method to include a chance of
 mutation (we'll default to 5%):
 */
void random_selection_with_mutation(population_type& p, double pmutate=0.05) {
    random_shuffle(p.begin(), p.end());
    population_type offspring(p.begin(), p.begin()+static_cast<std::size_t>(0.05*population_size));
    
    // mutate:
    for(population_type::iterator i=offspring.begin(); i!=offspring.end(); ++i) {
        if(pmutate < (static_cast<double>(rand())/RAND_MAX)) {
            mutate(*i);
        }
    }
    
    p.insert(p.end(), offspring.begin(), offspring.end());
    random_shuffle(p.begin(), p.end());
    p.erase(p.begin()+population_size, p.end());
}

#endif

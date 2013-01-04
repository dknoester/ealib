#ifndef _TUTORIAL_2_H_
#define _TUTORIAL_2_H_

/* Bring in the representation and fitness function from part 1:
 */
#include "tutorial_1.h"

/* Now, we need to start on the evolutionary part.  Let's begin with a population:
 */
typedef vector<repr_type> population_type;

/* This is simply a vector of repr_types.
 
 We also need to know how large this population should be, so let's define
 another constant for that purpose:
 */
size_t population_size=100;

/* Next, we need to decide what kind of generational model we'll use.  For this
 example, let's go with something simple.  We'll call it "random_selection":
 */
void random_selection(population_type& p) {
    random_shuffle(p.begin(), p.end());
    population_type offspring(p.begin(), p.begin()+static_cast<std::size_t>(0.05*population_size));
    
    p.insert(p.end(), offspring.begin(), offspring.end());
    random_shuffle(p.begin(), p.end());
    p.erase(p.begin()+population_size, p.end());
}
/* This generational model selects a random 5% for replication, copies them
 into an offspring population, merges the offspring into the original population,
 and then truncates the lot back down to a configured population size.
 */
 
#endif

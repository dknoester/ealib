#ifndef _EA_GENERATIONAL_MODELS_DEATH_GROWTH_PROCESS_H_
#define _EA_GENERATIONAL_MODELS_DEATH_GROWTH_PROCESS_H_

#include <algorithm>
#include <cmath>
#include <ea/interface.h>
#include <ea/generational_model.h>

namespace ea {
    
	namespace generational_models {
		
        struct no_growth_tag { };
        struct exponential_growth_tag { };
        
		/*! Death-birth process generational model.

		 The idea here is that all individuals that are slated to die (according
         to a configurable replacement rate) die at once, and then the population
         expands back to that size via fitness proportional selection (with replacement).
         
         This is a reasonable approximation of the Moran process, as described by Patrick Moran.
         
         \warning Fitness can not be negative.
		 */
        template <typename GrowthType=no_growth_tag>
		struct death_birth_process : public generational_model {
            typedef GrowthType growth_type;
            
            double adjust_fitness_sum(double fsum, double f, no_growth_tag) {
                return fsum;
            }

            double adjust_fitness_sum(double fsum, double f, exponential_growth_tag) {
                return fsum + f;
            }
            
			//! Apply this generational model to the EA to produce a single new generation.
			template <typename Population, typename EA>
			void operator()(Population& population, EA& ea) {
				BOOST_CONCEPT_ASSERT((PopulationConcept<Population>));
				BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
				
                // death (by virtue of not being copied into the next generation).
                // this can be done in a few different ways -- however, the only
                // correct way is one that ensures that each individual is tested
                // for death *once*, and that the selection appears to occur in
                // parallel:
                Population survivors;
                for(typename Population::iterator i=population.begin(); i!=population.end(); ++i) {
                    if(ea.rng().p(1.0-get<REPLACEMENT_RATE_P>(ea))) {
                        survivors.append(i);
                    }
                }
                std::swap(population, survivors);
                        
                // build a whole bunch of random numbers, and sort them:
                std::vector<double> rnums(get<POPULATION_SIZE>(ea) - population.size());
                std::generate(rnums.begin(), rnums.end(), ea.rng().uniform_real_rng(0,1.0));
                std::sort(rnums.begin(), rnums.end()); // ascending

                // sum of fitnesses:
                double fsum = accumulate_fitness(population.begin(), population.end(), ea);
                
                // now for the growth part.
                // we need to keep track of the sum of fitnesses as we append individuals to the
                // population.  since the rnums are sorted, we can scan through linearly.
                // since the fsum is monotonically increasing, all we need to do is scale
                // the current rnum by the fraction of fitness we're currently looking at:
                std::size_t p=0;
                double running=ind(population[p],ea).fitness();
                for(std::vector<double>::iterator i=rnums.begin(); i!=rnums.end(); ++i) {
                    double rnum=*i;
                    // while our running fitness is strictly less than the current
                    // random number, go to the next individual in the population:
                    while((running/fsum) < (rnum)) {
                        ++p;
                        assert(p<population.size());
                        running += ind(population[p],ea).fitness();
                    }
                    
                    // ok, running fitness is >= random number; select the p'th
                    // individual for replication:
                    Population parent, offspring;
                    parent.append(population[p]);
                    
                    recombine(parent, offspring, recombination::asexual(), ea);                    
                    mutate(ind(offspring.begin(),ea), ea);
                    calculate_fitness(ind(offspring.begin(),ea),ea);
                    population.append(offspring.begin());
                    
                    // update our fsum, and go 'round again:
                    fsum = adjust_fitness_sum(fsum, ind(offspring.begin(),ea).fitness(), growth_type());
                }
			}
		};
		
	} // generational_models
} // ea

#endif

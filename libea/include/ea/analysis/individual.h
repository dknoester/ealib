#ifndef _EA_ANALYSIS_MAX_FIT_INDIVIDUAL_H_
#define _EA_ANALYSIS_MAX_FIT_INDIVIDUAL_H_

#include <iostream>
#include <ea/meta_data.h>
#include <ea/analysis/tool.h>

namespace ea {
    namespace analysis {

        /*! Retest an individual.
         */
        template <typename EA>
        void test_individual(EA& ea) {
            typename EA::individual_type indi=individual_load(get<ANALYSIS_INPUT>(ea), ea);
            recalculate_fitness(indi,ea);
            individual_save(datafile(get<ANALYSIS_OUTPUT>(ea)), indi, ea);
        }
        // LIBEA_ANALYSIS_TOOL(test_individual, "test an individual")
        
        
        template <typename EA>
        typename EA::individual_type& find_most_fit_individual(EA& ea) {
            typename EA::population_type& pop = ea.population();
            typename EA::population_type::iterator mi=pop.begin();            
            for(typename EA::population_type::iterator i=pop.begin(); i!=pop.end(); ++i) {
                if(ind(i,ea).fitness() > ind(mi,ea).fitness()) {
                    mi = i;
                }
            }
            return ind(mi,ea);
        }
        
        
        /*! Retrieve an individual with the greatest fitness.
         */
        template <typename EA>
        void most_fit_individual(EA& ea) {
            individual_save(datafile(get<ANALYSIS_OUTPUT>(ea)), find_most_fit_individual(ea), ea);
        }        
        // LIBEA_ANALYSIS_TOOL(most_fit_individual, "select an individual with maximal fitness")

    } // analysis
} // ea

#endif

#ifndef _EA_ANALYSIS_POPULATION_H_
#define _EA_ANALYSIS_POPULATION_H_

#include <ea/datafile.h>
#include <ea/interface.h>


namespace ea {
    namespace analysis {

        /*! Save the fitness values of all individuals in the population.
         */
        template <typename EA>
        struct population_fitness : public ea::analysis::unary_function<EA> {
            static const char* name() { return "population_fitness"; }
            
            virtual void operator()(EA& ea) {
                
                datafile df(get<ANALYSIS_OUTPUT>(ea));
                for(typename EA::population_type::iterator i=ea.population().begin(); i!=ea.population().end(); ++i) {
                    df.write(ind(i,ea).fitness());
                }
                df.endl();
                
            }
        };
        
    } // analysis
} // ea

#endif

/* landscape.h 
 * 
 * This file is part of EALib.
 * 
 * Copyright 2012 David B. Knoester.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _EA_ANALYSIS_LANDSCAPE_H_
#define _EA_ANALYSIS_LANDSCAPE_H_

#include <iostream>
#include <sstream>
#include <cmath>
#include <vector>
#include <ea/datafile.h>

#include <ea/analysis/tool.h>


namespace ea {
    namespace analysis {

        /*! Sample the fitness landscape, and write fitness values to a file.
         */
        template <typename EA>
        void sample_landscape(EA& ea) {
            ea.population().clear();
            fitness_type_n(ea.population(), get<POPULATION_SIZE>(ea), ea);
            
            datafile df(get<ANALYSIS_OUTPUT>(ea));
            checkpoint_save(ea, df);
        }
        // LIBEA_ANALYSIS_TOOL(sample_landscape, "sample the fitness landscape")
        
        
        /*! Returns true if the given individual is on a local fitness peak.
         */
        template <typename EA>
        bool is_peak(typename EA::individual_type& indi, EA& ea) {
            typedef typename EA::fitness_type ftype;
            typedef typename EA::representation_type::codon_type ctype;
            
            // get a copy:
            typename EA::individual_type ind(indi);
            typename EA::representation_type& repr=representation(ind,ea);
        
            ftype w = recalculate_fitness(ind,ea); // wildtype fitness
        
            for(std::size_t i=0; i<repr.size(); ++i) {
                ctype a=repr[i];
                repr[i] = !a;
                ftype f = recalculate_fitness(ind,ea);
                repr[i] = a;
                if(f > w) {
                    return false;
                }
            }
            return true;
        }

        
//        /*! Attempt to sample peaks from the fitness landscape.
//         */
//        template <typename EA>
//        void sample_fitness_peaks(EA& ea) {
//            ea.population().clear();
//            
//            unsigned int samples=0;
//            while((ea.population().size() < get<POPULATION_SIZE>(ea)) && (samples < 100000000)) {
//                typename EA::population_entry_type s=generate_individual(ea);
//                samples++;
//                
//                if(is_peak(ind(s.second,ea),ea)) {
//                    datafile df(get<ANALYSIS_OUTPUT>(ea) + "/ind", samples, ".xml");
//                    boost::archive::xml_oarchive oa(df);
//                    oa << BOOST_SERIALIZATION_NVP(ind(s.second,ea));
//                    ea.population().insert(s);
//                }
//            }
//            datafile df(get<ANALYSIS_OUTPUT>(ea) + "/fitness_peaks.xml");
//            checkpoint_save(ea, df);
//        }
//        // LIBEA_ANALYSIS_TOOL(sample_fitness_peaks, "sample peaks from the fitness landscape")
        
        
        /*! Attempt to find fitness peaks via hill climbing.
         */
        template <typename EA>
        void hillclimb_fitness_peaks(EA& ea) {
            typedef typename EA::fitness_type ftype;
            typedef typename EA::representation_type::codon_type ctype;
            ea.population().clear();
            
            while(ea.population().size() < get<POPULATION_SIZE>(ea)) {                
                typename EA::population_entry_type s=generate_individual(ea);                
                typename EA::representation_type& repr=representation(ind(s.second,ea),ea);

                while(!is_peak(ind(s.second,ea),ea)) {
                    ftype w = recalculate_fitness(ind(s.second,ea),ea); // wildtype fitness
                
                    for(std::size_t i=0; i<repr.size(); ++i) {
                        ctype a=repr[i];
                        repr[i] = !a;
                        ftype f = recalculate_fitness(ind(s.second,ea),ea);
                        // revert the mutation if fitness is worse:
                        if(f < w) {
                            repr[i] = a;
                        }
                    }
                }
                ea.population().insert(s);
            }
            
            datafile df(get<ANALYSIS_OUTPUT>(ea));
            checkpoint_save(ea, df);
        }
        // LIBEA_ANALYSIS_TOOL(hillclimb_fitness_peaks, "find fitness peaks via hillclimbing")        
        
    } // analysis
} // ea

#endif

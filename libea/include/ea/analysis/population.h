/* analysis/population.h 
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
#ifndef _EA_ANALYSIS_POPULATION_H_
#define _EA_ANALYSIS_POPULATION_H_

#include <boost/lexical_cast.hpp>
#include <ea/datafile.h>

namespace ealib {
    namespace analysis {

        //! Save the fitness values of all individuals in the population.
        template <typename EA>
        struct unary_population_fitness : public ealib::analysis::unary_function<EA> {
            static const char* name() { return "unary_population_fitness"; }
            
            virtual void operator()(EA& ea) {
                datafile df("unary_population_fitness.dat");
                df.add_field("individual").add_field("fitness");
                for(typename EA::iterator i=ea.begin(); i!=ea.end(); ++i) {
                    df.write(i->name()).write(static_cast<double>(ealib::fitness(*i,ea))).endl();
                }
            }
        };
        
        //! Save the fitness values of all individuals in the population.
        template <typename EA>
        struct multivalued_population_fitness : public ealib::analysis::unary_function<EA> {
            static const char* name() { return "multivalued_population_fitness"; }
            
            virtual void operator()(EA& ea) {
                datafile df("multivalued_population_fitness.dat");
                df.add_field("individual");
                for(std::size_t i=0; i<ea.fitness_function().size(); ++i) {
                    df.add_field("objective_" + boost::lexical_cast<std::string>(i));
                }
                
                for(typename EA::iterator i=ea.begin(); i!=ea.end(); ++i) {
                    df.write(i->name());
                    for(std::size_t j=0; j<ea.fitness_function().size(); ++j) {
                        df.write(static_cast<double>(ealib::fitness(*i,ea)[j]));
                    }
                    df.endl();
                }
            }
        };
        
    } // analysis
} // ea

#endif

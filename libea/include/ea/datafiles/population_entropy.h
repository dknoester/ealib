/* population_entropy.h
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
#ifndef _EA_DATAFILES_POPULATION_ENTROPY_H_
#define _EA_DATAFILES_POPULATION_ENTROPY_H_

#include <boost/lexical_cast.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <ea/analysis.h>
#include <ea/datafile.h>


namespace ealib {
    namespace datafiles {
        
        /*! Datafile for genotypic entropy within a population.
         */
        template <typename EA>
        struct population_entropy : record_statistics_event<EA> {
            population_entropy(EA& ea) : record_statistics_event<EA>(ea), _df("population_entropy.dat") {
                _df.add_field("update")
                .add_field("entropy");
            }
            
            virtual ~population_entropy() {
            }
            
            virtual void operator()(EA& ea) {
                // measure genotypic entropy:
                std::vector<std::string> genotypes;
                for(typename EA::iterator i=ea.begin(); i!=ea.end(); ++i) {
                    genotypes.push_back(algorithm::vcat(i->repr().begin(), i->repr().end()));
                }
                _df.write(ea.current_update())
                .write(analysis::entropy(genotypes.begin(), genotypes.end()))
                .endl();
            }
            
            datafile _df;
        };

        /*! Datafile for genotypic entropy within a meta-population.
         */
        template <typename EA>
        struct meta_population_entropy : record_statistics_event<EA> {
            meta_population_entropy(EA& ea) : record_statistics_event<EA>(ea), _df("population_entropy.dat") {
                _df.add_field("update")
                .add_field("entropy");
            }
            
            virtual ~meta_population_entropy() {
            }
            
            virtual void operator()(EA& ea) {
                std::vector<std::string> genotypes;

                for(typename EA::iterator i=ea.begin(); i!=ea.end(); ++i) {
                    for(typename EA::individual_type::iterator j=i->begin(); j!=i->end(); ++j) {
                        genotypes.push_back(algorithm::vcat(j->repr().begin(), j->repr().end()));
                    }
                }
                
                _df.write(ea.current_update())
                .write(analysis::entropy(genotypes.begin(), genotypes.end()))
                .endl();
            }
            
            datafile _df;
        };

    } // datafiles
} // ea

#endif

/* adaptive_hfc.h 
 * 
 * This file is part of EALib.
 * 
 * Copyright 2012 David B. Knoester, Heather J. Goldsby.
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
#ifndef _EA_ADAPTIVE_HFC_H_
#define _EA_ADAPTIVE_HFC_H_

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <vector>
#include <limits>
#include <cmath>
#include <ea/datafile.h>
#include <ea/events.h>
#include <ea/meta_data.h>

namespace ealib {
    // ea.adaptive_hfc*
    // Initialization period - time to calibrate all the levels 
    // (called nCalibGen in paper)
    LIBEA_MD_DECL(INITIALIZATION_PERIOD, "ea.adaptive_hfc.initialization_period", unsigned int);
    // Period between times at which individuals are exchanged among subpops. 
    // (called nExch in paper)
    LIBEA_MD_DECL(EXCHANGE_INDIVIDUALS_PERIOD, "ea.adaptive_hfc.exchange_individuals_period", unsigned int);
    // Period between when admission levels are recalculated
    // (called nUpdateAdmissions in paper)
    LIBEA_MD_DECL(ADMISSION_UPDATE_PERIOD, "ea.adaptive_hfc.admission_update_period", unsigned int);
    // Minimum fraction of the population that is "left behind."
    LIBEA_MD_DECL(MIN_REMAIN, "ea.adaptive_hfc.min_remain", double);
    
    
    LIBEA_MD_DECL(ADMISSION_LEVEL, "ea.adaptive_hfc.admission_level", double);
    
    
    /*! Adaptive HFC migration among populations in a meta-population EA.
     Adapted from: Adaptive Hierarchical Fair Competition (AHFC) Model for Parallel 
     Evolutionary Algorithms by Jianjun Hu, Erik Goodman, Kisung Seo, and Min Pei
     */
    template <typename EA>
    struct adaptive_hfc : end_of_update_event<EA> {
        //! Constructor.
        adaptive_hfc(EA& ea) : end_of_update_event<EA>(ea) {
        }
        
        //! Destructor.
        virtual ~adaptive_hfc() {
        }
        
        //! Perform A-HFC migration among populations.
        virtual void operator()(EA& ea) {
            // Are we done with the initialization period? 
            if (ea.current_update() < get<INITIALIZATION_PERIOD>(ea)) return;
            
            // Are we in the initialization period?
            if (ea.current_update() == get<INITIALIZATION_PERIOD>(ea)) {
                setAdmissionLevels(ea);
            } else {
                if (ea.current_update() % get<ADMISSION_UPDATE_PERIOD>(ea) == 0) {
                    resetAdmissionLevels(ea);
                }
                if (ea.current_update() % get<EXCHANGE_INDIVIDUALS_PERIOD>(ea) == 0) {
                    exchangeIndividuals(ea);
                }
            }
        }
        
        void setAdmissionLevels(EA& ea) { 
            // Set up the admission levels. 
            // Go through all individuals in all subpops, record mean, max, std (of pop containing max)            
            using namespace boost::accumulators;
            
            accumulator_set<double, stats<tag::mean> > fit; 
            
            for(typename EA::iterator i=ea.begin(); i!=ea.end(); ++i) {
                for(typename EA::individual_type::population_type::iterator j=i->population().begin(); j!=i->population().end(); ++j) {
                    fit(static_cast<double>(ealib::fitness(**j,*i)));
                }
            }
            
            put<ADMISSION_LEVEL>(-std::numeric_limits<double>::infinity(), ea[0]);
            put<ADMISSION_LEVEL>(mean(fit), ea[1]);
            
            resetAdmissionLevels(ea);
        }
        
        void resetAdmissionLevels(EA& ea) {
            using namespace boost::accumulators;
            accumulator_set<double, stats<tag::mean, tag::max, tag::variance> > fit; 
            
            for(typename EA::individual_type::population_type::iterator j=ea.rbegin()->population().begin(); j!=ea.rbegin()->population().end(); ++j) {
                fit(static_cast<double>(ealib::fitness(**j,*ea.rbegin())));
            }
            
            double mean_base_one_fit = get<ADMISSION_LEVEL>(ea[1]);
            double max_fit = max(fit);
            double stddev_fit = sqrt(variance(fit));
            
            // Highest admission level is set to max - standard deviation 
            put<ADMISSION_LEVEL>(max_fit-stddev_fit, *ea.rbegin());
            
            // Other admission levels...
            int number_levels = get<META_POPULATION_SIZE>(ea);
            
            double per_level=max_fit - stddev_fit - mean_base_one_fit;
            assert(per_level > 0);
            per_level = std::max(1.0,per_level);
            
            for (int k = 2; k < (number_levels - 1); ++k) {
                put<ADMISSION_LEVEL>(mean_base_one_fit + static_cast<double>(k) * per_level / static_cast<double>(number_levels-2), ea[k]);
            }
        }
        
        // this should be recursive...
        void exchangeIndividuals(EA& ea) {
            // "moving" an individual is insert/erase between populations
            // "copying" an individual is insert(make_population_entry(ind))
            int number_levels = get<META_POPULATION_SIZE>(ea);
            for (int i = 0; i < (number_levels - 1); ++i) {
                double next_admission = get<ADMISSION_LEVEL>(ea[i+1]);
                
                // sort ascending by fitness:
                std::sort(ea[i].population().begin(), ea[i].population().end(), comparators::fitness<EA>(ea));

                // find the first individual w/ fitness >= next_admission, but make sure to leave some behind
                typename EA::individual_type::population_type::iterator f=ea[i].population().begin();
                std::advance(f, static_cast<std::size_t>(get<MIN_REMAIN>(ea)*get<POPULATION_SIZE>(ea)));
                typename EA::individual_type::population_type::iterator l=ea[i].population().end();
                for( ; f!=l; ++f) {
                    if(ealib::fitness(**f,ea[i]) > next_admission) {
                        break;
                    }
                }
                
                // now, move all individuals w/ fitness >= next_admission to the next pop:
                ea[i+1].population().insert(ea[i+1].population().end(), f,l);
                // and remove them from this one:
                ea[i].population().erase(f,l);
            }
            
            ea[0].initial_population();
        }
    };
    
    /*! Datafile for mean generation, and mean & max fitness.
     */
    template <typename EA>
    struct adaptive_hfc_datafile : record_statistics_event<EA> {
        adaptive_hfc_datafile(EA& ea) : record_statistics_event<EA>(ea), _df("ahfc.dat") {
            _df.add_field("update");
            for(std::size_t i=0; i<get<META_POPULATION_SIZE>(ea); ++i) {
                std::string prefix = "sp" + boost::lexical_cast<std::string>(i);
                _df.add_field(prefix + "_admission_level")
                .add_field(prefix + "_mean_fitness")
                .add_field(prefix + "_max_fitness");
            }
        }
        
        virtual ~adaptive_hfc_datafile() {
        }
        
        virtual void operator()(EA& ea) {
            using namespace boost::accumulators;
            
            _df.write(ea.current_update());
            
            for(std::size_t i=0; i<get<META_POPULATION_SIZE>(ea); ++i) {
                accumulator_set<double, stats<tag::mean> > age;
                accumulator_set<double, stats<tag::mean, tag::max> > fit;

                _df.write(get<ADMISSION_LEVEL>(ea[i], 0.0));
                          
                for(typename EA::individual_type::population_type::iterator j=ea[i].population().begin(); j!=ea[i].population().end(); ++j) {
                    fit(static_cast<double>(ealib::fitness(**j,ea[i])));
                }
                
                _df.write(mean(fit))
                .write(max(fit));
            }
            
            _df.endl();
        }
        
        datafile _df;
    };

} // ea

#endif

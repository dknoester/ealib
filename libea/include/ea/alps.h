/* alps.h 
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
#ifndef _EA_ALPS_H_
#define _EA_ALPS_H_


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

// update age - birth time
//     age = cur update - birth time -- a potential optimization.
// "reboot" lowest population
// parent selection scheme

namespace ea {
    // ea.alps*
    
    LIBEA_MD_DECL(GM_AGE, "ea.alps.genetic_material_age", unsigned int);
    LIBEA_MD_DECL(ADMISSION_AGE, "ea.alps.admission_age", double);
    
    template <typename MEA>
    struct alps : event {
        typedef typename MEA::individual_type EA;
        
        alps(MEA& ea) {
            _inheritance_conn.resize(get<META_POPULATION_SIZE>(ea));
            
            for(std::size_t i=0; i<get<META_POPULATION_SIZE>(ea); ++i) {
                _inheritance_conn[i] = ea[i].events().inheritance.connect(boost::bind(&alps::inheritance, this, _1, _2, _3));
                put<ADMISSION_AGE>(i*100, ea[i]);
            }
            
            _update_conn = ea.events().end_of_update.connect(boost::bind(&alps::end_of_update, this, _1));
        }
        
        /*! Called for every inheritance event. 
         */
        virtual void inheritance(typename EA::population_type& parents,
                                 typename EA::individual_type& offspring,
                                 EA& ea) {
            std::sort(parents.begin(), parents.end(), comparators::meta_data<GM_AGE>());
            put<GM_AGE>(get<GM_AGE>(*parents.back(),-1)+1, offspring);
        }
        
        //! Perform alps migration among populations.
        virtual void end_of_update(MEA& ea) {
            // increment the age of all individuals
            for(typename MEA::iterator i=ea.begin(); i!=ea.end(); ++i) {
                for(typename MEA::individual_type::population_type::iterator j=i->population().begin(); j!=i->population().end(); ++j) {
                    get<GM_AGE>(**j) += 1; 
                }
            }
            
            // "moving" an individual is insert/erase between populations
            // "copying" an individual is insert(make_population_entry(ind))
            for(int i=get<META_POPULATION_SIZE>(ea)-2; i>=0; --i) {
                double next_admission = get<ADMISSION_AGE>(ea[i+1]);

                // sort ascending by age:
                std::sort(ea[i].population().begin(), ea[i].population().end(), comparators::meta_data<GM_AGE>());
                
                // find the first individual w/ age >= next_admission:
                typename MEA::individual_type::population_type::iterator f=ea[i].population().begin();
                typename MEA::individual_type::population_type::iterator l=ea[i].population().end();
                for( ; f!=l; ++f) {
                    if (get<GM_AGE>(**f) > next_admission) {
                        break;
                    }
                }
                
                // now, move all individuals w/ fitness >= next_admission AND fitness greater
                // than min to the next pop:
                std::sort(ea[i+1].population().begin(), ea[i+1].population().end(), comparators::fitness());
                for(typename MEA::individual_type::population_type::iterator tf=f; tf!=l; ++tf) {
                    if((ea[i+1].population().size() < get<POPULATION_SIZE>(ea[i+1]))
                       || (ea::fitness(**tf) > ea::fitness(*ea[i+1].population().front()))) {
                        ea[i+1].population().insert(ea[i+1].population().end(), *tf);
                    }
                }
                // and remove them from this one:
                ea[i].population().erase(f,l);
            }
            
            ea[0].generate_initial_population();
        }
        
        std::vector<boost::signals::scoped_connection> _inheritance_conn;
        boost::signals::scoped_connection _update_conn;
    };
    
    
    /*! Datafile for mean generation, and mean & max fitness.
     */
    template <typename EA>
    struct alps_datafile : record_statistics_event<EA> {
        alps_datafile(EA& ea) : record_statistics_event<EA>(ea), _df("alps.dat") {
            _df.add_field("update");
            for(std::size_t i=0; i<get<META_POPULATION_SIZE>(ea); ++i) {
                std::string prefix = "sp" + boost::lexical_cast<std::string>(i);
                _df.add_field(prefix + "_mean_age")
                .add_field(prefix + "_mean_fitness")
                .add_field(prefix + "_max_fitness");
            }
        }
        
        virtual ~alps_datafile() {
        }
        
        virtual void operator()(EA& ea) {
            using namespace boost::accumulators;
            
            _df.write(ea.current_update());
            
            for(std::size_t i=0; i<get<META_POPULATION_SIZE>(ea); ++i) {
                if(ea[i].population().empty()) {
                    _df.write(0.0).write(0.0).write(0.0);
                } else {
                    accumulator_set<double, stats<tag::mean> > age;
                    accumulator_set<double, stats<tag::mean, tag::max> > fit;
                    
                    for(typename EA::individual_type::population_type::iterator j=ea[i].population().begin(); j!=ea[i].population().end(); ++j) {
                        age(get<GM_AGE>(**j,0.0));
                        fit(static_cast<double>(ea::fitness(**j)));
                    }
                    
                    _df.write(mean(age))
                    .write(mean(fit))
                    .write(max(fit));
                }
            }
            
            _df.endl();
        }
        
        datafile _df;
    };
    
} // ea

#endif

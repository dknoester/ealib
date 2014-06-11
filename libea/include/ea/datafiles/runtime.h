/* runtime.h
 *
 * This file is part of EALib.
 *
 * Copyright 2014 David B. Knoester.
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
#ifndef _EA_RUNTIME_H_
#define _EA_RUNTIME_H_

#include <sys/resource.h>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/timer.hpp>
#include <iostream>
#include <iomanip>

#include <ea/events.h>
#include <ea/fitness_function.h>

namespace ealib {
    namespace datafiles {

        /*! Output simple per-update run statistics.
         */
        template <typename EA>
        struct runtime : end_of_update_event<EA> {
            runtime(EA& ea) : end_of_update_event<EA>(ea) {
                std::cout << "update instantaneous_t average_t memory_usage" << std::endl;
                _t.restart();
            }
            
            virtual ~runtime() {
            }
            
            virtual void operator()(EA& ea) {
                using namespace boost::accumulators;
                double t=_t.elapsed();
                _tacc(t);
                std::cout << ea.current_update() << " ";
                
//                accumulator_set<double, stats<tag::mean> > gen;
//                accumulator_set<double, stats<tag::min, tag::mean, tag::max> > fit;
//                for(typename EA::population_type::iterator i=ea.population().begin(); i!=ea.population().end(); ++i) {
//                    gen((*i)->generation());
//                    fit(static_cast<double>(ealib::fitness(**i,ea)));
//                }
                
//                std::cout << mean(gen) << " " << min(fit) << " " << mean(fit) << " " << max(fit) << " ";
                std::cout << std::fixed << std::setprecision(4) << t << " ";
                std::cout << std::fixed << std::setprecision(4) << boost::accumulators::mean(_tacc) << " ";
                
                double rss=1024.0;
#ifdef __APPLE__
                rss *= 1024.0; // bug in apple documentation; ru_maxrss is in units of bytes.
#endif
                
                rusage r;
                getrusage(RUSAGE_SELF, &r);
                std::cout << std::fixed << std::setprecision(4) << r.ru_maxrss/rss << std::endl;
                
                _t.restart();
            }
                
            boost::timer _t;
            boost::accumulators::accumulator_set<double, boost::accumulators::stats<boost::accumulators::tag::mean> > _tacc;
        };
        
        /*! Output simple per-update run statistics.
         */
        template <typename EA>
        struct emscript : end_of_update_event<EA> {
            emscript(EA& ea) : end_of_update_event<EA>(ea) {
                std::cout << "update instantaneous_t average_t population_size mean_generation mean_priority" << std::endl;
                _t.restart();
            }
            
            virtual ~emscript() {
            }
            
            virtual void operator()(EA& ea) {
                using namespace boost::accumulators;
                double t=_t.elapsed();
                _tacc(t);

                std::cout << ea.current_update() << " ";
                std::cout << std::fixed << std::setprecision(4) << t << " ";
                std::cout << std::fixed << std::setprecision(4) << boost::accumulators::mean(_tacc) << " ";
                std::cout << ea.size() << " ";
                
                accumulator_set<double, stats<tag::mean> > gen;
                accumulator_set<double, stats<tag::mean> > priority;
                for(typename EA::iterator i=ea.begin(); i!=ea.end(); ++i) {
                    gen(get<IND_GENERATION>(*i));
                    priority(static_cast<double>(i->priority()));
                }

                std::cout << mean(gen) << " " << mean(priority) << std::endl;
                _t.restart();
            }
            
            boost::timer _t;
            boost::accumulators::accumulator_set<double, boost::accumulators::stats<boost::accumulators::tag::mean> > _tacc;
        };
    } // datafiles
} // ea
                
#endif

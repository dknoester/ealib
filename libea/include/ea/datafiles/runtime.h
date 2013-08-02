/* runtime.h
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
#ifndef _EA_RUNTIME_H_
#define _EA_RUNTIME_H_

#include <sys/resource.h>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/timer.hpp>
#include <iostream>

#include <ea/events.h>

namespace ealib {
    namespace datafiles {

        /*! Output simple per-update run statistics.
         */
        template <typename EA>
        struct runtime : end_of_update_event<EA> {
            runtime(EA& ea) : end_of_update_event<EA>(ea) {
                _t.restart();
            }
            
            virtual ~runtime() {
            }
            
            virtual void operator()(EA& ea) {
                double t=_t.elapsed();
                _tacc(t);
                std::cerr << std::fixed << std::setprecision(4) << t << " ";
                std::cerr << std::fixed << std::setprecision(4) << boost::accumulators::mean(_tacc) << " ";
                
                double rss=1024.0;
#ifdef __APPLE__
                rss *= 1024.0; // bug in apple documentation; ru_maxrss is in units of bytes.
#endif
                
                rusage r;
                getrusage(RUSAGE_SELF, &r);
                std::cerr << std::fixed << std::setprecision(4) << r.ru_maxrss/rss << std::endl;
                
                
                _t.restart();
            }
                
            boost::timer _t;
            boost::accumulators::accumulator_set<double, boost::accumulators::stats<boost::accumulators::tag::mean> > _tacc;
        };
    } // datafiles
} // ea
                
#endif

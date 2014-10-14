/* stopping.h
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
#ifndef _EA_STOPPING_H_
#define _EA_STOPPING_H_

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/max.hpp>

#include <ea/traits.h>
#include <ea/metadata.h>

namespace ealib {
    
    /* The function objects here provide "early stopping" criteria for an EA.
     
     Normally, an EA is run for a specified number of updates.  These functors
     provide a means to stop such an EA early, e.g., if stagnation is detected.
     */

    //! Don't stop early; default behavior.
    struct dont_stop {
        template <typename EA>
        bool operator()(EA& ea) {
            return false;
        }
    };
    
    
    LIBEA_MD_DECL(STOP_STAGNANT_UPDATES, "ea.stop.stagnant_n", unsigned int);

    /*! Function object that stops the EA when the max fitness has not improved
     for X updates.
     */
    struct stagnant_updates {

        //! Constructor.
        stagnant_updates() : _last_improvement(0) {
        }
        
        template <typename EA>
        bool operator()(EA& ea) {
            using namespace boost::accumulators;
            accumulator_set<double, stats<tag::max> > fit;
            
            for(typename EA::iterator i=ea.begin(); i!=ea.end(); ++i) {
                fit(static_cast<double>(ealib::fitness(*i,ea)));
            }

            if((_last_improvement == 0) || (_maxf < boost::accumulators::max(fit))) {
                _last_improvement = ea.current_update();
                _maxf = boost::accumulators::max(fit);
            }
            
            if((ea.current_update() - _last_improvement) > get<STOP_STAGNANT_UPDATES>(ea)) {
                return true;
            }
            return false;
        }
        
        unsigned long _last_improvement; //!< Last update of a max fitness improvement.
        double _maxf; //!< Last max fitness.
    };
    
} // ea

#endif

/* sigma_scaling.h
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
#ifndef _EA_SELECTION_SIGMA_SCALING_H_
#define _EA_SELECTION_SIGMA_SCALING_H_

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/sum.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/variance.hpp>

#include <ea/attributes.h>
#include <ea/selection.h>

namespace ealib {
    LIBEA_MD_DECL(SIGMA_SCALING_M, "ea.selection.sigma_scaling.m", double);

	namespace selection {
        
		/*! Proportionate selection via sigma-scaling.
         
         This strategy selects individuals proportionately by some attribute,
         but scales the selection probability by the individual's relationship to
         the mean and standard deviation of the population with respect to that 
         attribute.
         
         Sigma-scaling has the benefit of maintaining a relatively consistent
         selection pressure throughout evolution.
		 */
        template <typename AttributeAccessor=access::fitness>
		struct sigma_scaling {
            typedef AttributeAccessor acc_type; //!< Accessor for proportionate selection.
            
			//! Initializing constructor.
			template <typename Population, typename EA>
			sigma_scaling(std::size_t n, Population& src, EA& ea) : _mu(0.0), _sigma(0.0) {
                using namespace boost::accumulators;
                
                accumulator_set<double, stats<tag::mean,tag::variance> > w;
                for(typename Population::iterator i=src.begin(); i!=src.end(); ++i) {
                    w(static_cast<double>(_acc(**i,ea)));
                }
                
                _sigma=sqrt(variance(w));
                _mu=mean(w);
			}
            
			/*! Select n individuals via sigma-scaling proportionate selection.
             */
			template <typename Population, typename EA>
			void operator()(Population& src, Population& dst, std::size_t n, EA& ea) {
                // build a whole bunch of random numbers, and sort them:
                std::vector<double> rnums(n);
                std::generate(rnums.begin(), rnums.end(), ea.rng().uniform_real_rng(0,1.0));
                std::sort(rnums.begin(), rnums.end()); // ascending
                
                // scale the selection attribute of all individuals in the population
                // based on mean and sd; negative means P(selection) == 0
                std::vector<double> w(src.size());
                std::size_t j=0;
                double sum=0.0;
                for(typename Population::iterator i=src.begin(); i!=src.end(); ++i,++j) {
                    // f(v) = max(v - (mu - m*sigma), 0.0)
                    double v = std::max(static_cast<double>(_acc(**i,ea)) - (_mu - get<SIGMA_SCALING_M>(ea)*_sigma), 0.0);
                    w[j] = v;
                    sum += v;
                }
                
                double offset=0.0;
                if(sum == 0.0) {
                    sum = static_cast<double>(src.size());
                    offset = 1.0;
                }
                
                // since the rnums are sorted, we can scan through linearly.
                // since the fsum is monotonically increasing, all we need to do is scale
                // the current rnum by the fraction of fitness we're currently looking at:
                j=0;
                double running=w[j] + offset;
                
                for(std::vector<double>::iterator i=rnums.begin(); i!=rnums.end(); ++i) {
                    // while the fraction of running fitness is strictly less than
                    // the current random number, go to the next individual in
                    // the population:
                    while((running/sum) < (*i)) {
                        ++j;
                        assert(j!=w.size());
                        running += w[j] + offset;
                    }
                    
                    // ok, running fitness is >= random number; select the p'th
                    // individual for replication:
                    dst.push_back(src[j]);
                }
            }
            
            acc_type _acc; //!< Accessor for value used for proportionate selection.
            double _mu; //!< Mean fitness.
            double _sigma; //!< Standard deviation of fitness.
		};
        
	} // selection
} // ealib

#endif

/* digital_evolution/groups.h 
 * 
 * This file is part of EALib.
 * 
 * Copyright 2014 David B. Knoester, Heather J. Goldsby.
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

#ifndef _EA_DIGITAL_EVOLUTION_GROUPS_H_
#define _EA_DIGITAL_EVOLUTION_GROUPS_H_

#include <algorithm>
#include <ea/mutation.h>
#include <ea/metadata.h>
#include <ea/selection/random.h>
#include <ea/selection/proportionate.h>
#include <ea/datafile.h>


namespace ealib {
    
    /*! An event that performs periodic competition among metapopulations, based upon
     some attribute accessor.
     */
    template <typename AttributeAccessor, typename EA>
    struct meta_population_competition : periodic_event<METAPOP_COMPETITION_PERIOD,EA> {
        /*! Constructor.
         
         \param order this parameter defines the order in which this event will 
         be executed relative to other events that are scheduled to execute as a
         result of the same signal.  For example, if an event is used to trigger
         calculation of the attribute that will be used for selection by this event.
         Larger numbers occur later.
         */
        meta_population_competition(int order, EA& ea) : periodic_event<METAPOP_COMPETITION_PERIOD,EA>(order, ea)
        , _df("meta_population_competition.dat") {
            _df.add_field("update").add_field("mean_fitness").add_field("max_fitness");
        }
        
        //! Destructor.
        virtual ~meta_population_competition() {
        }
        
        //! Called every METAPOP_COMPETITION_PERIOD updates to perform meta-population competition.
        virtual void operator()(EA& ea) {
            // save some quick stats...
            using namespace boost::accumulators;
            accumulator_set<double, stats<tag::mean, tag::max> > fit;
            AttributeAccessor acc;
            for(typename EA::iterator i=ea.begin(); i!=ea.end(); ++i) {
                fit(acc(*i));
            }
            _df.write(ea.current_update()).write(mean(fit)).write(max(fit)).endl();
            
            // how many survivors?
            std::size_t n = static_cast<std::size_t>((1.0 - get<REPLACEMENT_RATE_P>(ea)) * get<METAPOPULATION_SIZE>(ea));
            
            // select individuals for survival:
            typename EA::population_type survivors;
            select_n<selection::random>(ea.population(), survivors, n, ea);
            
            // how many offspring?
            n = get<METAPOPULATION_SIZE>(ea) - survivors.size();
            
            // select the parents:
            typename EA::population_type parents;
            select_n<selection::proportionate<AttributeAccessor> >(survivors, parents, n, ea);
            
            // now, recombine each parent to produce an offspring (population):
            typename EA::population_type offspring;
            for(typename EA::population_type::iterator i=parents.begin(); i!=parents.end(); ++i) {
                typename EA::individual_ptr_type p=ea.make_individual();
                germline_replication(**i, *p, ea);
                offspring.push_back(p);
            }
            
            // reset the survivors:
            for(typename EA::population_type::iterator i=survivors.begin(); i!=survivors.end(); ++i) {
                (*i)->reset();
            }
            
            // add the offspring to the list of survivors:
            survivors.insert(survivors.end(), offspring.begin(), offspring.end());
            
            // and swap 'em in for the current population:
            std::swap(ea.population(), survivors);
        }
        
        datafile _df; //!< Datafile produced by meta-population competition.
    };
    
    
    /*! This group replication method fills the offspring group with copies of a
     single mutated individual from the parent group.
     
     This works best when groups are assumed to be genetically homogeneous.
     */
    template <typename EA>
    void germline_replication(typename EA::individual_type& parent, typename EA::individual_type& offspring, EA& ea) {
        // grab a copy of the first individual:
        typename EA::individual_type::individual_type germ(**parent.population().begin());
        
        // mutate it:
        mutate(germ, offspring);
        
        // and now fill up the offspring population with copies of the germ:
        for(std::size_t j=0; j<get<POPULATION_SIZE>(offspring); ++j) {
            offspring.append(offspring.make_individual(germ.repr()));
        }        
    } 
    
} // ea

#endif

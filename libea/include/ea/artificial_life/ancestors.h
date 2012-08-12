/* artificial_life/ancestors.h 
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

#ifndef _EA_ARTIFICIAL_LIFE_ANCESTORS_H_
#define _EA_ARTIFICIAL_LIFE_ANCESTORS_H_

#include <boost/serialization/nvp.hpp>
#include <boost/shared_ptr.hpp>

#include <ea/concepts.h>
#include <ea/configuration.h>
#include <ea/artificial_life/hardware.h>
#include <ea/artificial_life/isa.h>
#include <ea/artificial_life/organism.h>
#include <ea/artificial_life/schedulers.h>
#include <ea/artificial_life/replication.h>
#include <ea/artificial_life/well_mixed.h>
#include <ea/artificial_life/task_library.h>
#include <ea/initialization.h>
#include <ea/interface.h>

#include <ea/mutation.h>
#include <ea/population.h>
#include <ea/recombination.h>
#include <ea/events.h>
#include <ea/rng.h>

#include <ea/meta_data.h>


namespace ea {
    
    /*! Generates the default ancestor.
     */
    struct repro_ancestor {
        template <typename EA>
        typename EA::population_entry_type operator()(EA& ea) {
            typedef typename EA::representation_type representation_type;
            typename EA::individual_type ind;
            ind.name() = next<INDIVIDUAL_COUNT>(ea);
            ind.repr().resize(get<REPRESENTATION_SIZE>(ea));
            representation_type& repr=ind.repr();
            
            std::fill(ind.repr().begin(), ind.repr().end(), ea.isa()["nop_x"]);
            *ind.repr().rbegin() = ea.isa()["repro"];
            return make_population_entry(ind, ea);
        }
    };
    
    /*! Generates the nop-x ancestor.
     */
    struct nopx_ancestor {
        template <typename EA>
        typename EA::population_entry_type operator()(EA& ea) {
            typedef typename EA::representation_type representation_type;
            typename EA::individual_type ind;
            ind.name() = next<INDIVIDUAL_COUNT>(ea);
            
            representation_type& repr=ind.repr();
            repr.resize(get<REPRESENTATION_SIZE>(ea));
            std::fill(repr.begin(), repr.end(), ea.isa()["nop_x"]);
            return make_population_entry(ind, ea);
        }
    };
    
    /*! Generates the self-replicator ancestor.
     */
    struct selfrep_ancestor {
        template <typename EA>
        typename EA::population_entry_type operator()(EA& ea) {
            typedef typename EA::representation_type representation_type;
            typename EA::individual_type ind;
            ind.name() = next<INDIVIDUAL_COUNT>(ea);
            
            representation_type& repr=ind.repr();
            repr.resize(get<REPRESENTATION_SIZE>(ea));
            std::fill(repr.begin(), repr.end(), ea.isa()["nop_x"]);
            
            // Must use representation size of 100.
            assert(repr.size() == 100);
            
            repr[0] =  ea.isa()["h_alloc"]; // h_alloc
            repr[1] =  ea.isa()["nop_c"]; // nopc
            repr[2] =  ea.isa()["nop_a"]; // nopa
            repr[3] =  ea.isa()["h_search"]; // hsearch
            repr[4] =  ea.isa()["nop_c"]; // nopc
            repr[5] =  ea.isa()["mov_head"]; // movhead
            
            repr[91] =  ea.isa()["h_search"]; // hsearch
            repr[92] =  ea.isa()["h_copy"]; // hcopy
            repr[93] =  ea.isa()["nop_c"]; // nopc
            repr[94] =  ea.isa()["nop_a"]; // nopa
            repr[95] =  ea.isa()["if_label"]; // iflabel
            repr[96] =  ea.isa()["h_divide"]; // hdivide
            repr[97] =  ea.isa()["mov_head"]; // movhead
            repr[98] =  ea.isa()["nop_a"]; // nopa
            repr[99] =  ea.isa()["nop_b"]; // nopb
            
            ind.hw().initialize();
            
            return make_population_entry(ind, ea);
        }
    };

    
} // ea

#endif

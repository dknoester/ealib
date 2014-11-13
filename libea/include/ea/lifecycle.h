/* lifecycle.h
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
#ifndef _EA_LIFECYCLE_H_
#define _EA_LIFECYCLE_H_

#include <ea/checkpoint.h>
#include <ea/metadata.h>

namespace ealib {
    
	LIBEA_MD_DECL(RUN_UPDATES, "ea.run.updates", int);
	LIBEA_MD_DECL(RUN_EPOCHS, "ea.run.epochs", int);
    
    /*! At the conceptual level, this class defines the states and actions of
     an EA's lifecycle.  Where appropriate, these states correspond to methods
     defined below.
     
     object construction
     |          |
     v          |
     load       |
     |          |
     v          v
     meta-data assignment
     |          |
     v          v
     initialize (final resource preparation, event attachment, etc.)
     |          |
     **after_initialization()**
     |          |
     |    initial_population
     |          |
     |    **after_initial_population()**
     |          |
     v          v
     begin epoch
     |
     v
     update <-N-- stop? --Y-> end epoch ---> save
          \___ /
     */
    struct default_lifecycle {
        
        /*! Called after EA initialization.
         
         This is a good place to handle programmatic setup tasks.  E.g., adding
         instructions to a digital evolution ISA, loading external data files,
         and the like.
         
         Note that there is also an "after_initialization" method in the
         cmdline_interface, which is perhaps more convenient.  This method
         should really only be used when you need to do something strange.
         */
        template <typename EA>
        void after_initialization(EA& ea) {
        }
        
        /*! Called after the initial population has been generated.
         
         See note re: cmdline_interface above; the same holds true here.
         */
        template <typename EA>
        void after_initial_population(EA& ea) {
        }
        
        //! Called to reset the state of this population.
        template <typename EA>
        void reset(EA& ea) {
        }

        //! Advance the EA by one epoch of n updates.
        template <typename EA>
        void advance_epoch(int n, EA& ea) {
            ea.begin_epoch();
            for( ; n>0; --n) {
                ea.update();
                if(ea.stop()) {
                    break;
                }
            }
            ea.end_epoch();
        }

        //! Advance the EA by one epoch.
        template <typename EA>
        void advance_epoch(EA& ea) {
            advance_epoch(get<RUN_UPDATES>(ea), ea);
        }
        
        //! Advance the EA by all configured epochs, and checkpoint after each.
        template <typename EA>
        void advance_all(EA& ea) {
            if(exists<RUN_EPOCHS>(ea)) {
                for(int i=0; i<get<RUN_EPOCHS>(ea); ++i) {
                    advance_epoch(ea);
                    checkpoint::save(ea);
                }
            } else {
                do {
                    advance_epoch(ea);
                    checkpoint::save(ea);                    
                } while(!ea.stop());
            }
		}
    };
    
} // ealib

#endif

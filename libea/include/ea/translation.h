/* translation.h
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
#ifndef _EA_TRANSLATION_H_
#define _EA_TRANSLATION_H_

namespace ealib {
    namespace translators {
        
        /*! This translator is used to "reconstruct" a phenotype from a genome.
         
         It can be thought of as a *very* simple form of indirect encoding, where 
         the phenotype is constructable from the genome.
         
         In other words, the phenotype != genome_type, but we don't need a full-blown
         translator.
         */
        struct construction {
            template <typename EA>
            construction(EA& ea) {
            }
            
            template <typename EA>
            typename EA::phenotype_type operator()(typename EA::genome_type& g, EA& ea) {
                return typename EA::phenotype_type(g, ea);
            }
        };
        
    } // translators
} // network

#endif

/* neuroevolution.h
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
#ifndef _ANN_NEUROEVOLUTION_H_
#define _ANN_NEUROEVOLUTION_H_

#include <ann/feed_forward.h>
#include <ea/metadata.h>

namespace ann {
    
    LIBEA_MD_DECL(ANN_INPUT_N, "neural_network.input.n", std::size_t);
    LIBEA_MD_DECL(ANN_OUTPUT_N, "neural_network.output.n", std::size_t);
    
    /*! This translator is used to "reconstruct" an ANN from a genome.  It's a
     simple way to get around EALib's requirement that genome_types be default
     constructable, without needing a full-blown genome translator.
     */
    struct neural_network_reconstruction {
        template <typename EA>
        neural_network_reconstruction(EA& ea) {
        }
        
        template <typename EA>
        typename EA::phenotype_type operator()(typename EA::genome_type& g, EA& ea) {
            return typename EA::phenotype_type(ealib::get<ANN_INPUT_N>(ea) + ealib::get<ANN_OUTPUT_N>(ea), g.begin());
        }
    };
    
} // network

#endif

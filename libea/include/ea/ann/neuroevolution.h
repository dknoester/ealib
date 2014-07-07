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
#ifndef _EA_ANN_NEUROEVOLUTION_H_
#define _EA_ANN_NEUROEVOLUTION_H_

#include <ea/metadata.h>
#include <ea/mutation.h>

namespace ealib {
    
    LIBEA_MD_DECL(ANN_INPUT_N, "ea.ann.input.n", std::size_t);
    LIBEA_MD_DECL(ANN_OUTPUT_N, "ea.ann.output.n", std::size_t);
    LIBEA_MD_DECL(ANN_HIDDEN_N, "ea.ann.hidden.n", std::size_t);
    
	namespace mutation {
		namespace operators {
			
			/*! Mutates the weight matrix of a neural network only (does not
			 alter size).
			 */
			struct weight_matrix {
				template <typename EA>
				void operator()(typename EA::individual_type& ind, EA& ea) {
					typename EA::genome_type& N=ind.genome();
					const std::size_t n = N.size();
					const double per_site_p=get<MUTATION_PER_SITE_P>(ea);
					
					for(std::size_t i=0; i<n; ++i) {
						for(std::size_t j=0; j<n; ++j) {
							if(ea.rng().p(per_site_p)) {
								N(i,j) = ea.rng().normal_real(N(i,j), get<MUTATION_NORMAL_REAL_VAR>(ea));
							}
						}
					}
				}
			};
			
		} // operators
	} // mutation
    
	namespace ancestors {
		
		//! Generates neural networks with random weight matrices.
		struct random_weight_neural_network {
			template <typename EA>
			typename EA::genome_type operator()(EA& ea) {
				typename EA::genome_type N(get<ANN_INPUT_N>(ea),
										   get<ANN_OUTPUT_N>(ea),
										   get<ANN_HIDDEN_N>(ea));
				const std::size_t n = N.size();
				for(std::size_t i=0; i<n; ++i) {
					for(std::size_t j=0; j<n; ++j) {
						N(i,j) = ea.rng().normal_real(0.0, get<MUTATION_NORMAL_REAL_VAR>(ea));
					}
				}
				return N;
			}
		};
		
	} // ancestors
} // ealib

#endif

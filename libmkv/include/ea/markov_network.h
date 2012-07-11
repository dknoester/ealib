/* markov_network.h
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
#ifndef _FN_MARKOV_NETWORK_EA_H_
#define _FN_MARKOV_NETWORK_EA_H_

#include <ea/interface.h>
#include <ea/meta_data.h>
#include <ea/analysis.h>
#include <mkv/markov_network.h>

namespace ea {
	
	/*! Markov network mutation type.
     
     Performs per-site, duplication, and deletion mutations.
	 */
	struct mkv_mutation {
		template <typename Representation, typename EA>
		void operator()(Representation& repr, EA& ea) {
			typedef typename std::vector<typename Representation::codon_type> codon_buffer;
			
			double per_site_p = get<MUTATION_PER_SITE_P>(ea);
            int imax = get<MUTATION_UNIFORM_INT_MAX>(ea);
			for(typename Representation::iterator i=repr.begin(); i!=repr.end(); ++i) {
				if(ea.rng().p(per_site_p)) {
					*i = ea.rng()(imax);
				}
			}
			
            // gene duplication
            // (the below looks a little crude, but there were some problems related
            // to incorrect compiler optimization.)
			if(ea.rng().p(get<MUTATION_DUPLICATION_P>(ea)) && (repr.size()<20000)) {
                int start = ea.rng().uniform_integer(0, repr.size());
                int extent = ea.rng().uniform_integer(16, 513);
                codon_buffer buf(extent);
                for(int i=0; i<extent; ++i) {
                    buf[i] = repr[start+i];
                }                
				repr.insert(ea.rng().choice(repr.begin(),repr.end()), buf.begin(), buf.end());
			}
            
            // gene deletion
			if(ea.rng().p(get<MUTATION_DELETION_P>(ea)) && (repr.size()>1000)) {
				int start, extent;
				extent = 15+ea.rng()(512);
				start = ea.rng()(repr.size()-extent);
				repr.erase(repr.begin()+start, repr.begin()+start+extent);
			}
		}
	};
	
	
	/*! Generates random Markov network-based individuals.
	 */
	struct mkv_random_individual {
        template <typename EA>
        typename EA::population_entry_type operator()(EA& ea) {
            typedef typename EA::representation_type representation_type;
            typename EA::individual_type ind;
            ind.name() = next<INDIVIDUAL_COUNT>(ea);
            ind.repr().resize(get<REPRESENTATION_SIZE>(ea), 127);            
            representation_type& repr=ind.repr();
			
			int i,j;
			for(i=0; i<get<MKV_INITIAL_NODES>(ea); ++i) {
				j=ea.rng()(repr.size()-100);
                int gate=ea.rng()(3);
                repr[j] = 42+gate;
                repr[j+1] = (255-42-gate);
				for(int k=2; k<20; ++k) {
					repr[j+k]=ea.rng()(256);
				}
			}
			return make_population_entry(ind,ea);
		}
	};
    
} // ea

#endif

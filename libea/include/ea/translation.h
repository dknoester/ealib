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

#include <utility>
#include <vector>

namespace ealib {
    
    //! Helper method to aid in translation of genome -> phenotype.
    template <typename Genome, typename Phenotype, typename Translator, typename EA>
    void translate(Genome& G, Phenotype& P, Translator t, EA& ea) {
        t(G,P,ea);
    }

	
	template <typename Genome, typename Phenotype>
	struct abstract_gene {
		virtual void operator()(typename Genome::iterator f, Phenotype& P) = 0;
	};

	
    /*! Generic translator class to aid in translating a genome to a phenotype.
     */
	template <typename Genome, typename Phenotype>
    class translator {
    public:
        typedef Genome genome_type;
        typedef Phenotype phenotype_type;
        typedef abstract_gene<genome_type, phenotype_type> abstract_gene_type;
		typedef boost::shared_ptr<abstract_gene_type> gene_ptr_type;
		typedef std::vector<gene_ptr_type> gene_list_type;
		
		
        //! Constructor.
        translator() {
        }
		
		//! Translate genome G into phenotype P.
		template <typename EA>
		void operator()(genome_type& G, phenotype_type& P, EA& ea) {
			for(typename genome_type::iterator i=G.begin(); i!=G.end(); ++i) {
				if(((*i + *(i+1)) == 255) && (*(i+1) < _genes.size())) {
					(*_genes[*(i+1)])(i+2, P, ea);
				}
			}
		}

		template <typename Gene, typename EA>
		void add_gene(EA& ea) {
			gene_ptr_type p(new Gene(ea));
			_genes.push_back(p);
		}
		
	protected:
		gene_list_type _genes;
    };
    

	

//        /*! This translator is used to "reconstruct" a phenotype from a genome.
//         
//         It can be thought of as a *very* simple form of indirect encoding, where 
//         the phenotype is constructable from the genome.
//         
//         In other words, the phenotype != genome_type, but we don't need a full-blown
//         translator.
//         */
//        struct construction {
//            template <typename EA>
//            construction(EA& ea) {
//            }
//            
//            template <typename EA>
//            typename EA::phenotype_type operator()(typename EA::genome_type& g, EA& ea) {
//                return typename EA::phenotype_type(g, ea);
//            }
//        };
//        
//    } // translators
} // network

#endif

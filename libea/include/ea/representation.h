/* representation.h
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
#ifndef _EA_REPRESENTATION_H_
#define _EA_REPRESENTATION_H_

#include <ea/metadata.h>
#include <ea/translation.h>

namespace ealib {
    
    LIBEA_MD_DECL(REPRESENTATION_SIZE, "ea.representation.size", int);
    LIBEA_MD_DECL(REPRESENTATION_INITIAL_SIZE, "ea.representation.initial_size", int);
	LIBEA_MD_DECL(REPRESENTATION_MIN_SIZE, "ea.representation.min_size", int);
	LIBEA_MD_DECL(REPRESENTATION_MAX_SIZE, "ea.representation.max_size", int);
    
    /* We are defining a {\em representation} as a (genome_type, phenotype_type,
     translator) tuple.
     
     The idea here is that some kinds of EAs require that an individual's genotype
     be converted to another form prior to fitness evaluation.  This is usually
     referred to as the "encoding type."  Common encoding types are direct (each 
     codon in the genome directly corresponds to a phenotypic feature), indirect 
     (the phenotype must be translated from the genome), generative (the phenotype
     must be produced by the genome), and developmental (the phenotype is "grown"
     from the genome, and can change during the individual's lifetime).
     
     \note generative and development are not yet supported.
     */
    
    //! Tag indicating that the individual's genome directly encodes the phenotype.
    struct directS { };
    
    /*! Tag indicating that the individaul's genome indirectly encodes the phenotype.
     */
    struct indirectS { };
    
    //! Tag indicating that the individual's phenotype must be generated (not yet supported).
    struct generativeS { };

    //! Tag indicating that the individual's phenotype must be developed from the genome (not yet supported).
    struct developmentalS { };

    
    /*! Direct representation type.
     
     Here, the phenotype is the same as the genome type.
     */
    template <typename Genome>
    struct direct {
        typedef Genome genome_type;
        typedef genome_type phenotype_type;
        typedef directS encoding_type;
        
        //! Constructor.
        direct() { }
        
        //! Copy constructor.
        direct(const direct& d) : _genome(d._genome) {
        }
        
        //! Constructor.
        direct(const genome_type& g) : _genome(g) {
        }
        
        //! Assignment operator.
        direct& operator=(const direct& that) {
            if(this != &that) {
                _genome = that._genome;
            }
            return *this;
        }
        
        //! Returns a reference to the genome.
        genome_type& genome() { return _genome; }
        
        //! Returns a reference to the phenotype (==genome).
        template <typename EA>
        phenotype_type& phenotype(EA& ea) { return _genome; }
        
        //! Serialize this representation.
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("genome", _genome);
        }
        
        genome_type _genome; //!< Genome for this representation.
    };
    
    
    /*! Indirect representation type.
     
     Here, the phenotype is calculated from the genome type.
     */
    template <typename Genome, typename Phenotype, typename Translator>
    struct indirect {
        typedef Genome genome_type;
        typedef Phenotype phenotype_type;
        typedef boost::shared_ptr<phenotype_type> phenotype_ptr_type;
        typedef indirectS encoding_type;
        typedef Translator translator_type;

        //! Constructor.
        indirect() { }
        
        //! Copy constructor.
        indirect(const indirect& d) : _genome(d._genome) {
        }
        
        //! Constructor.
        indirect(const genome_type& g) : _genome(g) {
        }
        
        //! Assignment operator.
        indirect& operator=(const indirect& that) {
            if(this != &that) {
                _genome = that._genome;
            }
            return *this;
        }
        
        //! Returns a reference to the genome.
        genome_type& genome() { return _genome; }
        
        //! Returns a reference to the phenotype (==genome).
        template <typename EA>
        phenotype_type& phenotype(EA& ea) {
            if(_phenotype == 0) {
                _phenotype.reset(new phenotype_type());
                translate(_genome, *_phenotype, translator_type(ea), ea);
            }
            return *_phenotype;
        }
        
        //! Serialize this representation.
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("genome", _genome);
        }
        
        genome_type _genome; //!< Genome for this representation.
        phenotype_ptr_type _phenotype; //!< Phenotype for this representation.
    };

    //! Convenience method that returns a reference to an individual's phenotype.
    template <typename EA>
    typename EA::phenotype_type& phenotype(typename EA::individual_type& ind, EA& ea) {
        return ind.repr().phenotype(ea);
    }
    
} // ea

#endif

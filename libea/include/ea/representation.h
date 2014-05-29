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


namespace ealib {
    
    /* We are defining a {\em representation} as a (genome, phenotype, 
     translation code) tuple.
     
     The idea here is that some kinds of EAs require that an individual's genotype
     be converted to another form prior to fitness evaluation.  This is usually
     referred to as the "encoding type."
     
     Here we define two different encoding types: direct and indirect.
     */
    
    //! Indicates that the individual's genotype directly encodes the phenotype.
    struct directS { };
    
    /*! Indicates that the individaul's genotype indirectly encodes the phenotype
     (i.e., must be translated or generated prior to fitness evaluation).
     */
    struct indirectS { };
    
    /* Generative encodings are not yet defined, though they will likely be added
     later.
     
     These different encoding types are used when make_phenotype() is called
     on an individual.  Note that the default is directS.
     
     To be clear about definitions:
     
     Genotype: That which makes up the genetic component of an individual; inherited.
     In EALib, genotypes == representation.
     
     Phenotype: That component of an individual that is evaluated by the fitness
     function; can be the genotype (direct), an object that was translated
     from the genotype (indirect), or even an object that was generated
     (generative).
     */

    
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
            if(_phenotype != 0) {
                translator_type t(ea);
                _phenotype.reset(t(_genome,ea));
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

} // ea

#endif

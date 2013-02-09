/* line_of_descent.h
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
#ifndef _EA_LINE_OF_DESCENT_H_
#define _EA_LINE_OF_DESCENT_H_

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/regex.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <set>

#include <ea/datafile.h>
#include <ea/events.h>
#include <ea/individual.h>


namespace ea {

    /*! Wrapper class for individuals to enable line of descent (lod) tracking.
     
     A note about construction and assignment: We specifically *do not* copy the
     parent set.  If we did, then every offspring would have a pointer to its entire
     lineage.  While this is kind of interesting, it's unneeded overhead and artificially
     inflates reference counts.
     */
    template <typename Representation, typename Attributes>
	class lod_individual : public individual<Representation,Attributes> {
    public:
        typedef individual<Representation,Attributes> base_type;
        typedef boost::shared_ptr<lod_individual> lod_ptr_type;
        typedef std::set<lod_ptr_type> parent_set_type;

        //! Constructor.
        lod_individual() : base_type() {
        }

        //! Constructor.
        lod_individual(const Representation& r) : base_type(r) {
        }

        //! Copy constructor.
        lod_individual(const lod_individual& that) : base_type(that) {
        }
        
        //! Assignment operator.
        lod_individual& operator=(const lod_individual& that) {
            if(this != & that) {
                base_type::operator=(that);
                _lod_parents.clear();
            }
            return *this;
        }
        
        //! Destructor.
        virtual ~lod_individual() {
        }
        
        //! Retrieve the set of this individual's parents.
        parent_set_type& lod_parents() { return _lod_parents; }
        
        //! Shorthand for asexual populations.
        lod_ptr_type lod_parent() { return *_lod_parents.begin(); }
        
        //! Returns true if this individual has parents.
        bool has_parents() { return (_lod_parents.size() > 0); }
        
    protected:
        parent_set_type _lod_parents; //!< This individual's set of parents.
    };

    
    /*! Chains together offspring and their parents, called for every inheritance event.
     */
    template <typename EA>
    struct lod_event : inheritance_event<EA> {
        
        //! Constructor.
        lod_event(EA& ea) : inheritance_event<EA>(ea) {
        }
        
        //! Destructor.
        virtual ~lod_event() {
        }
        
        //! Called for every inheritance event.
        virtual void operator()(typename EA::population_type& parents,
                                typename EA::individual_type& offspring,
                                EA& ea) {
            for(typename EA::population_type::iterator i=parents.begin(); i!=parents.end(); ++i) {
                offspring.lod_parents().insert(*i);
            }
        }
    };

    
    /*! Contains line of descent information.
     
     This class holds information about a line of descent.  It does this by storing
     an internal lineage, which is initially empty.  Subsequent calls to class member
     functions (e.g., mrca_lineage()) alter this lineage.  As a result, this class
     can be serialized and/or copied for later analysis at any time.
     
     Note: Asexual only.
     */
    template <typename EA>
    class line_of_descent {
    public:
        typedef EA ea_type;
        typedef typename ea_type::individual_type individual_type;
        typedef typename ea_type::individual_ptr_type individual_ptr_type;
        typedef std::list<individual_ptr_type> lineage_type;
        typedef typename lineage_type::iterator iterator;
        typedef typename lineage_type::reverse_iterator reverse_iterator;
        
        //! Constructor.
        line_of_descent() {
        }
        
        //! Destructor.
        virtual ~line_of_descent() {
        }
        
        //! Returns the lineage.
        lineage_type& lineage() { return _lod; }
        
        iterator begin() { return _lod.begin(); }        
        iterator end() { return _lod.end(); }
        reverse_iterator rbegin() { return _lod.rbegin(); }        
        reverse_iterator rend() { return _lod.rend(); }

        //! Returns the size (number of genomes) on the current lineage.
        std::size_t size() const { return _lod.size(); }
        
        //! Calculate the most recent common ancestor's lineage.
        void mrca_lineage(ea_type& ea) {
            _lod = lineage(mrca(ea));
        }
        
        //! Remove the default ancestor
        void remove_default_ancestor() {
            _lod.erase(_lod.begin());
        }
        
        //! Erase the entry at the specified iterator.
        iterator erase(iterator pos) {
            return _lod.erase(pos);
        }
        
        //! Remove all redundant genomes from this lineage, preserving the most recent.
        void uniq() {
            if(_lod.size()>1) {
                typename lineage_type::iterator back=_lod.begin();
                typename lineage_type::iterator i=_lod.begin(); ++i;
                for( ; i!=_lod.end(); ++i) {
                    if((*i)->repr() == (*back)->repr()) {
                        _lod.erase(back);
                    }
                    back = i;
                }
            }
        }

        //! Remove all redundant genomes from this lineage, preserving the oldest.
        void runiq() {
            if(_lod.size()>1) {
                typename lineage_type::iterator back=_lod.end(); --back;
                typename lineage_type::iterator i=back; --i;

                for( ; i!=_lod.begin(); --i) { // stopping at begin is ok, as that's the ancestor
                    if((*i)->repr() == (*back)->repr()) {
                        _lod.erase(back);
                    }
                    back = i;
                }
            }
        }

    protected:
        /*! Calculate the lineage of the given individual.
         
         The lineage is ordered from ancestor to offspring.
         */
        lineage_type lineage(individual_ptr_type p) {
            lineage_type lod;
            
            lod.push_back(p);
            while(p->lod_parents().size()>0) {
                p = p->lod_parent();
                lod.push_front(p);
            }
            
            return lod;
        }
        
        /*! Calculate the most recent common ancestor (MRCA) of the current
         population.
         
         Because we're using ref counts on individuals, this is done in O(n) time,
         and proceeds as follows.  We start with any individual in the current population,
         and proceed backwards along its lineage.  Whenever we find a parent that has
         a smaller ref count than its offspring, we assign the current mrca to that
         offspring.  Also, whenever we find a parent that has a greater ref count than
         the offspring, we set the mrca to the parent (this is to handle the case where the
         mrca happens to be the progenitor).
         */
        individual_ptr_type mrca(ea_type& ea) {
            assert(ea.population().size()>0);
            individual_ptr_type offspring=*ea.population().begin();
            individual_ptr_type parent;
            individual_ptr_type m=offspring;
            
            while(offspring->has_parents()) {
                parent = offspring->lod_parent();
                
                if(parent.use_count() < offspring.use_count()) {
                    m = offspring;
                } else if(parent.use_count() > offspring.use_count()) {
                    m = parent;
                }

                offspring = parent;
            }            
            
            return m;
        }

        lineage_type _lod; //!< The current line of descent.
        
    private:
        friend class boost::serialization::access;

        template<class Archive>
		void save(Archive & ar, const unsigned int version) const {
            std::size_t s = _lod.size();
            ar & boost::serialization::make_nvp("lineage_size", s);
            for(typename lineage_type::const_iterator i=_lod.begin(); i!=_lod.end(); ++i) {
                ar & boost::serialization::make_nvp("individual", **i);
            }
		}
		
		template<class Archive>
		void load(Archive & ar, const unsigned int version) {
            std::size_t s;
            ar & boost::serialization::make_nvp("lineage_size", s);
            _lod.clear();
            for(std::size_t i=0; i<s; ++i) {
                individual_ptr_type p(new individual_type());
                ar & boost::serialization::make_nvp("individual", *p);
                _lod.push_back(p);
            }
		}

		BOOST_SERIALIZATION_SPLIT_MEMBER();
    };
    

    /*! Line-of-descent datafile.
     
     Saves the lod at the end of every epoch.
     */     
    template <typename EA>
    struct mrca_lineage_datafile : end_of_epoch_event<EA> {
       
        //! Constructor.
        mrca_lineage_datafile(EA& ea) : end_of_epoch_event<EA>(ea), _lod_event(ea) {
        }
        
        //! Destructor.
        virtual ~mrca_lineage_datafile() {
        }
        
        //! Called at the end of every epoch; saves the current lod.
        virtual void operator()(EA& ea) {
            line_of_descent<EA> lod;
            lod.mrca_lineage(ea);

            datafile df("lod", ea.current_update(), ".xml");
            boost::archive::xml_oarchive oa(df);            
            oa << BOOST_SERIALIZATION_NVP(lod);
        }

        lod_event<EA> _lod_event;
    };


    LIBEA_MD_DECL(FIXATION_TIME, "individual.fixation_time", long);
    
    /*! Tracks the update at which individuals along the line of descent have fixed
     in the population.
     
     Requires the that lod tracking be enabled.
     */     
    template <typename EA>
    struct track_fixation_events : end_of_update_event<EA> {
        
        //! Constructor.
        track_fixation_events(EA& ea) : end_of_update_event<EA>(ea) {
        }
        
        //! Destructor.
        virtual ~track_fixation_events() {
        }
        
        /*! Called at the end of every update to track fixation events.
         
         The idea here is that if we have individuals that are on the end of the LoD,
         and we haven't previously tagged them with a fixation time, do so now.  The
         tag is set to the current update.
         */
        virtual void operator()(EA& ea) {
            typedef line_of_descent<EA> lod_type;
            lod_type lod;
            lod.mrca_lineage(ea);
            
            for(typename lod_type::reverse_iterator i=lod.rbegin(); i!=lod.rend(); ++i) {
                if(!exists<FIXATION_TIME>(ind(i,ea))) {
                    put<FIXATION_TIME>(ea.current_update(), ind(i,ea));
                } else {
                    break;
                }
            }
        }
    };

    
    /*! Serialize a line of descent object.
     */
    template <typename EA>
    void lod_save(std::ostream& out, line_of_descent<EA>& lod, EA& ea) {
        boost::archive::xml_oarchive oa(out);
        oa << BOOST_SERIALIZATION_NVP(lod);        
    }
    
    
	/*! Load a previously serialized line of descent object.
     */
	template <typename EA>
	line_of_descent<EA> lod_load(std::istream& in, EA& ea) {
        line_of_descent<EA> lod;
		boost::archive::xml_iarchive ia(in);
		ia >> BOOST_SERIALIZATION_NVP(lod);
        return lod;
	}
    
    /*! Load a previously serialized line of descent object.
     */
    template <typename EA>
    line_of_descent<EA> lod_load(const std::string& fname, EA& ea) {
        std::ifstream ifs(fname.c_str());

        // is this a gzipped file?  test by checking file extension...
        static const boost::regex e(".*\\.gz$");
        if(boost::regex_match(fname, e)) {
            namespace bio = boost::iostreams;
            bio::filtering_stream<bio::input> f;
            f.push(bio::gzip_decompressor());
            f.push(ifs);
            return lod_load(f, ea);
        } else {
            return lod_load(ifs, ea);
        }
    }
    
} // ea

#endif

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

#include <boost/iterator/indirect_iterator.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/regex.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include <sstream>
#include <fstream>
#include <vector>
#include <string>

#include <ea/datafile.h>
#include <ea/events.h>
#include <ea/lifecycle.h>
#include <ea/individual.h>
#include <ea/digital_evolution/organism.h>

namespace ealib {
    namespace traits {
        
        /*! Line of descent (LoD) trait.
         */
        template <typename T>
        struct lod_trait {
            typedef typename T::individual_ptr_type individual_ptr_type;
            typedef std::vector<individual_ptr_type> parent_vector_type;
            
            //! Constructor.
            lod_trait() {
            }
            
            //! Clear this individual's parents.
            void lod_clear() {
                _lod_parents.clear();
            }

            //! Retrieve all of this individual's parents.
            parent_vector_type& lod_parents() { return _lod_parents; }
            
            //! Shorthand for asexual populations.
            individual_ptr_type lod_parent() {
                assert(!_lod_parents.empty());
                return _lod_parents.front();
            }
            
            //! Returns true if this individual has parents.
            bool has_parents() {
                return !_lod_parents.empty();
            }

            //! Serialize this LoD trait.
            template <class Archive>
            void serialize(Archive& ar, const unsigned int version) {
            }

            parent_vector_type _lod_parents; //!< Vector of pointers to this individual's parents.
        };
        
    } // traits
    
    
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
        //! Type of the EA this LoD is based on.
        typedef EA ea_type;
        //! Type for individuals in the LoD.
        typedef typename ea_type::individual_type individual_type;
        //! Type for pointers to individuals in the LoD.
        typedef typename ea_type::individual_ptr_type individual_ptr_type;
        //! Type for a lineage, a list of individuals.
        typedef std::list<individual_ptr_type> lineage_type;
        //! Iterator over this LoD's lineage.
        typedef boost::indirect_iterator<typename lineage_type::iterator> iterator;
        //! Const iterator over this LoD's lineage.
        typedef boost::indirect_iterator<typename lineage_type::const_iterator> const_iterator;
        //! Reverse iterator over this LoD's lineage.
        typedef boost::indirect_iterator<typename lineage_type::reverse_iterator> reverse_iterator;
        //! Const reverse iterator over this LoD's lineage.
        typedef boost::indirect_iterator<typename lineage_type::const_reverse_iterator> const_reverse_iterator;
        
        //! Constructor.
        line_of_descent() {
        }
        
        //! Destructor.
        virtual ~line_of_descent() {
        }
        
        //! Returns a begin iterator to the population.
        iterator begin() {
            return iterator(_lod.begin());
        }
        
        //! Returns an end iterator to the population.
        iterator end() {
            return iterator(_lod.end());
        }
        
        //! Returns a begin iterator to the population (const-qualified).
        const_iterator begin() const {
            return const_iterator(_lod.begin());
        }
        
        //! Returns an end iterator to the population (const-qualified).
        const_iterator end() const {
            return const_iterator(_lod.end());
        }
        
        //! Returns a reverse begin iterator to the population.
        reverse_iterator rbegin() {
            return reverse_iterator(_lod.rbegin());
        }
        
        //! Returns a reverse end iterator to the population.
        reverse_iterator rend() {
            return reverse_iterator(_lod.rend());
        }
        
        //! Returns a reverse begin iterator to the population (const-qualified).
        const_reverse_iterator rbegin() const {
            return const_reverse_iterator(_lod.rbegin());
        }
        
        //! Returns a reverse end iterator to the population (const-qualified).
        const_reverse_iterator rend() const {
            return const_reverse_iterator(_lod.rend());
        }
        
        //! Returns the size (number of individuals) on the current lineage.
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
            while(p->traits().lod_parents().size()>0) {
                p = p->traits().lod_parent();
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
            assert(ea.get_population().size()>0);
            individual_ptr_type offspring=*ea.get_population().begin();
            individual_ptr_type parent;
            individual_ptr_type m=offspring;
            
            while(offspring->traits().has_parents()) {
                parent = offspring->traits().lod_parent();
                
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
            ar & boost::serialization::make_nvp("size", s);
            for(typename lineage_type::const_iterator i=_lod.begin(); i!=_lod.end(); ++i) {
                ar & boost::serialization::make_nvp("individual", **i);
            }
		}
		
		template<class Archive>
		void load(Archive & ar, const unsigned int version) {
            std::size_t s;
            ar & boost::serialization::make_nvp("size", s);
            _lod.clear();
            for(std::size_t i=0; i<s; ++i) {
                individual_ptr_type p(new individual_type());
                ar & boost::serialization::make_nvp("individual", *p);
                _lod.push_back(p);
            }
		}
        
		BOOST_SERIALIZATION_SPLIT_MEMBER();
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
                offspring.traits().lod_parents().push_back(*i);
            }
        }
    };
    
    
    /*! Meta-population enabled LOD event.
     */
    template <typename EA>
    struct meta_population_lod_event : event {
        typedef lod_event<typename EA::individual_type> event_type;
        typedef boost::shared_ptr<event_type> ptr_type;
        typedef std::vector<ptr_type> event_list_type;
        
        //! Constructor.
        meta_population_lod_event(EA& ea) {
            for(typename EA::iterator i=ea.begin(); i!=ea.end(); ++i) {
                ptr_type p(new event_type(*i));
                _events.push_back(p);
            }
        }
        
        //! Destructor.
        virtual ~meta_population_lod_event() {
        }

        event_list_type _events;
    };
    
    
    namespace datafiles {
    
        //! Line-of-descent from the default ancestor to the current MRCA.
        template <typename EA>
        struct mrca_lineage : end_of_epoch_event<EA> {
            //! Constructor.
            mrca_lineage(EA& ea) : end_of_epoch_event<EA>(ea), _lod_event(ea) {
            }
            
            //! Destructor.
            virtual ~mrca_lineage() {
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
        
        /*! Meta-population enabled MRCA lineage datafile.
         */
        template <typename EA>
        struct meta_population_mrca_lineage : end_of_epoch_event<EA> {
            meta_population_mrca_lineage(EA& ea) : end_of_epoch_event<EA>(ea), _lod_event(ea) {
            }
            
            //! Destructor.
            virtual ~meta_population_mrca_lineage() {
            }
            
            //! Called at the end of every epoch; saves the current lod.
            virtual void operator()(EA& ea) {
                std::size_t count=0;
                for(typename EA::iterator i=ea.begin(); i!=ea.end(); ++i, ++count) {
                    line_of_descent<typename EA::individual_type> lod;
                    lod.mrca_lineage(*i);
                    
                    datafile df("sp" + boost::lexical_cast<std::string>(count) + "_lod", ea.current_update(), ".xml");
                    boost::archive::xml_oarchive oa(df);
                    oa << BOOST_SERIALIZATION_NVP(lod);
                }
            }
            
            meta_population_lod_event<EA> _lod_event;
        };
        
    } // datafiles
    
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

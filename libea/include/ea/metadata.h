/* metadata.h
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
#ifndef _EA_METADATA_H_
#define _EA_METADATA_H_

#include <wordexp.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/any.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/serialization/map.hpp>
#include <boost/program_options.hpp>

#include <string>
#include <map>

#include <ea/exceptions.h>

namespace ealib {
    
    //! ABC for attribute types.
    struct abstract_attribute {
        //! Convert this attribute's value to a string.
        virtual std::string to_string() = 0;
        
        //! Convert this attribute's value from a string.
        virtual void from_string(const std::string& v) = 0;
    };
    
    
    /*! This is the only specialization of abstract_attribute, and it provides
     the data-type specific components needed for string / value conversion.
     
     The other part that's needed is that attributes must provide a static key()
     method (so that we don't need an instance everytime we want to manipulate an
     attribute).  For now, this is handled via a macro, LIBEA_MD_DECL, which is
     down below a ways.
     */
    template <typename T>
    struct attribute : abstract_attribute {
        typedef T value_type; //!< Type of value stored in this attribute.
        typedef value_type& reference_type; //!< Reference type to the value.

        //! Constructor.
        attribute() {
        }
        
        virtual ~attribute() {
        }

        //! Convert this attribute's value to a string.
        virtual std::string to_string() { return boost::lexical_cast<std::string>(_value); }
        
        //! Convert this attribute's value from a string.
        virtual void from_string(const std::string& v) { _value = boost::lexical_cast<value_type>(v); }

        //! Return this attribute's value.
        reference_type value() { return _value; }
        
        value_type _value; //!< This attribute's value.
    };

    
	/*! Meta-data is a collection of string covertible key-value pairs (attributes).

	 The idea here is that meta-data is fundamentally a map of strings->strings, and
	 then we convert between the strings and the actual value_type on demand.  Doing
	 it this way makes serialization much easier, but more importantly, it lets us
	 define new pieces of meta-data without requiring nasty linking tricks (ie, exporting
	 each meta-data class to the serialization library).

     At serialization time, all extant attributes are converted into their string 
     form and serialized.
     
     At runtime, the string versions of attributes are lazily converted to their
     native representation.

	 Meta-data is used with the free functions get, put, exists, and next (which
     is a convenient test-and-inc).
     */
	class metadata {
	public:
		typedef std::map<std::string,std::string> md_string_type; //!< Container for the string versions of attributes
        typedef boost::shared_ptr<abstract_attribute> attr_ptr_type; //!< Type for pointer to attributes.
		typedef std::map<std::string,attr_ptr_type> md_value_type; //!< Container for the native versions of attributes.
		
		//! Constructor.
		metadata() { }
		
		//! Destructor.
		virtual ~metadata() { }
		
		//! Copy-constructor.
		metadata(const metadata& that) {
			_strings = that._strings;
            _values = that._values;
		}
		
		//! Assignment operator.
		metadata& operator=(const metadata& that) {
			if(this != &that) {
				_strings = that._strings;
                _values = that._values;
			}
			return *this;
		}
		
        //! Copy meta-data from that to this (may overwrite this).
        metadata& operator+=(const metadata& that) {
            if(this != &that) {
                flush();
                const_cast<metadata*>(&that)->flush();
                _values.clear();
                for(md_string_type::const_iterator i=that._strings.begin(); i!=that._strings.end(); ++i) {
					_strings[i->first] = i->second;
                }
            }
            return *this;
        }
        
        //! Operator==
        bool operator==(const metadata& that) const {
            return std::equal(_strings.begin(), _strings.end(), that._strings.begin());
        }
        
        //! Returns a reference to this meta-data object (for compatibiliy with the get & set methods below).
        metadata& md() { return *this; }
		
		//! Returns the string backing an attribute's value.
		template <typename Attribute>
		std::string& getstr(const std::string& k) {
			md_string_type::iterator j=_strings.find(k);
			if(j == _strings.end()) {
				throw uninitialized_metadata_exception(k);
			}
			return j->second;
		}
		
		//! Returns a reference to an attribute's value.
        template <typename Attribute>
        typename Attribute::reference_type getattr(const std::string& k) {
			md_value_type::iterator i=_values.find(k);
			if(i == _values.end()) {
				// cache miss:
				md_string_type::iterator j=_strings.find(k);
				if(j == _strings.end()) {
					throw uninitialized_metadata_exception(k);
				}
                // convert the attr:
                attr_ptr_type p(new Attribute());
                p->from_string(j->second);
				i = _values.insert(std::make_pair(k,p)).first;
			}
			return static_cast<Attribute*>(i->second.get())->value();
		}

        //! Returns a reference to an attribute's value.
        template <typename Attribute>
        typename Attribute::reference_type getattr(const std::string& k, const typename Attribute::value_type v) {
			md_value_type::iterator i=_values.find(k);
			if(i == _values.end()) {
				// cache miss:
                attr_ptr_type p(new Attribute());
				md_string_type::iterator j=_strings.find(k);
                if(j == _strings.end()) {
                    static_cast<Attribute*>(p.get())->value() = v;
                } else {
                    p->from_string(j->second);
                }
                i = _values.insert(std::make_pair(k,p)).first;
			}
			return static_cast<Attribute*>(i->second.get())->value();
		}

        //! Sets an attributes value.
        template <typename Attribute>
        typename Attribute::reference_type setattr(const std::string& k, const typename Attribute::value_type v) {
            md_value_type::iterator i=_values.find(k);
			if(i == _values.end()) {
                // build the attr:
                attr_ptr_type p(new Attribute());
				i = _values.insert(std::make_pair(k,p)).first;
			}
            Attribute* attr = static_cast<Attribute*>(i->second.get());
            attr->value() = v;
            return attr->value();
		}
        
        /*! Bypass type conversions, and store the string version of an attribute directly.

         This causes a string conversion on the first getattr call with this key, but is
         necessary because we don't actually know the type in question here.
         */
		void set(const std::string& k, const std::string& v) {
			_strings[k] = v;
            _values.erase(k);
		}

		//! Check to see if meta-data with key k exists.
		bool exists(const std::string& k) {
			md_value_type::iterator i=_values.find(k);
			if(i == _values.end()) {
				// cache miss:
				md_string_type::iterator j=_strings.find(k);
				if(j == _strings.end()) {
					return false;
				}
			}
			return true;
		}
		
		//! Clear all meta data.
		void clear() {
			_strings.clear();
			_values.clear();
		}
		
	private:			
		md_string_type _strings; //!< Container for meta-data.
		md_value_type _values; //!< Cache for meta-data (not ever serialized).
		
        //! Convert all values to strings.
        void flush() {
            for(md_value_type::iterator i=_values.begin(); i!=_values.end(); ++i) {
                _strings[i->first] = i->second->to_string();
            }
        }
        
		friend class boost::serialization::access;
		template <class Archive>
		void serialize(Archive& ar, const unsigned int version) {
            flush();
			ar & boost::serialization::make_nvp("metadata", _strings);
		}		
	};
    
    //! Returns a reference to the given attribute's value.
    template <typename Attribute, typename HasMetaData>
    typename Attribute::reference_type get(HasMetaData& hmd) {
        return hmd.md().template getattr<Attribute>(Attribute::key());
    }
    
    //! Returns a reference to the given attribute, setting it a default value if it is not present.
    template <typename Attribute, typename HasMetaData>
    typename Attribute::reference_type get(HasMetaData& hmd, const typename Attribute::value_type def) {
        return hmd.md().template getattr<Attribute>(Attribute::key(),def);
    }
    
    //! Sets the value of the given attribute, and returns a reference to it.
    template <typename Attribute, typename HasMetaData>
    typename Attribute::reference_type put(const typename Attribute::value_type v, HasMetaData& hmd) {
        return hmd.md().template setattr<Attribute>(Attribute::key(), v);
    }
    
    //! Sets the string for the given value.
    template <typename HasMetaData>
    void put(const std::string& k, const std::string& v, HasMetaData& hmd) {
        hmd.set(k,v);
    }    
    
    //! Returns true if the attribute exists, false otherwise.
    template <typename Attribute, typename HasMetaData>
    bool exists(HasMetaData& hmd) {
        return hmd.md().exists(Attribute::key());
    }
    
    //! Increment and set an attribute.
    template <typename Attribute, typename HasMetaData>
    typename Attribute::reference_type next(HasMetaData& hmd) {
        typename Attribute::reference_type v = get<Attribute>(hmd,-1);
        ++v;
        return v;
    }
	
    //! Convenience method to test a periodic condition against metadata.
    template <typename Attribute, typename HasMetaData>
    bool periodic(const typename Attribute::value_type v, HasMetaData& hmd) {
        return (v!=0) && ((v % get<Attribute>(hmd)) == 0);
    }

} // ea

/* This macro defines a new attribute. */
#define LIBEA_MD_DECL( name, key_string, type ) \
struct name : ealib::attribute<type> { \
    virtual ~name() { } \
    inline static const char* key() { return key_string; } \
}

namespace ealib {
	LIBEA_MD_DECL(IND_UNIQUE_NAME, "individual.unique_name", std::string);
    LIBEA_MD_DECL(IND_GENERATION, "individual.generation", double);
    LIBEA_MD_DECL(IND_BIRTH_UPDATE, "individual.birth_update", long);
    
    LIBEA_MD_DECL(POPULATION_SIZE, "ea.population.size", unsigned int);
    LIBEA_MD_DECL(METAPOPULATION_SIZE, "ea.metapopulation.size", unsigned int);

	// ea.representation.*
	LIBEA_MD_DECL(REALSTRING_MEAN, "ea.representation.realstring.mean", double);
	LIBEA_MD_DECL(REALSTRING_VARIANCE, "ea.representation.realstring.variance", double);
	LIBEA_MD_DECL(INTSTRING_MEAN, "ea.representation.intstring.mean", int);
	LIBEA_MD_DECL(INTSTRING_VARIANCE, "ea.representation.intstring.variance", int);
	LIBEA_MD_DECL(UINTSTRING_MEAN, "ea.representation.uintstring.mean", unsigned int);
	LIBEA_MD_DECL(UINTSTRING_VARIANCE, "ea.representation.uintstring.variance", unsigned int);
	
    // ea.environment.*
    LIBEA_MD_DECL(LOCATION_DATA, "ea.environment.location.color", int);
    
    // ea.statistics.*
    LIBEA_MD_DECL(RECORDING_PERIOD, "ea.statistics.recording.period", unsigned long);
        
} // ealib

#endif

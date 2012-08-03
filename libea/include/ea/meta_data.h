/* meta_data.h
 * 
 * This file is part of EALib.
 * 
 * Copyright 2012 David B. Knoester, Randal S. Olson.
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
#ifndef _EA_META_DATA_H_
#define _EA_META_DATA_H_

#include <boost/any.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/serialization/map.hpp>
#include <boost/program_options.hpp>
#include <string>
#include <map>

#include <ea/exceptions.h>

namespace ea {
	
	/*! This class provides objects with a way to store arbitrary meta-data.
     
     It supports two distinct concepts:

     1) Arbitrary string-convertible meta-data, which is serialized.
	 The idea here is that meta-data is fundamentally a map of strings->strings, and
	 then we convert between the strings and the actual value_type on demand.  Doing
	 it this way makes serialization much easier, but more importantly, it lets us
	 define new pieces of meta-data without requiring nasty linking tricks (ie, exporting
	 each meta-data class to the serialization library).
     
     Use these with "get", "put", "next", and "exists".
     
     2) Arbitrary attributes, which are not serialized.
     This enables us to temporarily attach attributes to meta-data supporting objects.
     These attributes are not serialized, thus they do not need to be string convertible.
     For example, one such attribute might be the parent of an individual, or the distance
     in phenotype space between individuals.
     
     Use these with "getattr", "setattr", and "rmattr".
     
     ** NOT YET IMPLEMENTED.
     
	 */
	class meta_data {
	public:
		typedef std::map<std::string,std::string> meta_data_container; //!< Container for meta-data elements.
		typedef std::map<std::string,boost::any> meta_data_cache; //!< Cache for meta-data to prevent extra string conversions.
		
		//! Constructor.
		meta_data() { }
		
		//! Destructor.
		virtual ~meta_data() { }
		
		//! Copy-constructor.
		meta_data(const meta_data& that) {
			_meta_data = that._meta_data;
            _cache.clear();
		}
		
		//! Assignment operator.
		meta_data& operator=(const meta_data& that) {
			if(this != &that) {
				_meta_data = that._meta_data;
                _cache.clear();
			}
			return *this;
		}
		
		//! Bypass type conversions, and store meta-data directly.
		template <typename V>
		void put(const std::string& k, const V& v) {
			_meta_data[k] = boost::lexical_cast<std::string>(v);
            _cache.erase(k);
		}
		
		//! Retrieve meta-data.
		template <typename V>
		V get(const std::string& k) {
			meta_data_cache::iterator i=_cache.find(k);
			if(i == _cache.end()) {
				// cache miss:
				meta_data_container::iterator j=_meta_data.find(k);
				if(j == _meta_data.end()) {
					throw uninitialized_meta_data_exception(k);
				}
				i = _cache.insert(std::make_pair(k, boost::lexical_cast<V>(j->second))).first;
			}
			return boost::any_cast<V>(i->second);
		}
		
		//! Check to see if meta-data with key k exists.
		bool exists(const std::string& k) {
			meta_data_cache::iterator i=_cache.find(k);
			if(i == _cache.end()) {
				// cache miss:
				meta_data_container::iterator j=_meta_data.find(k);
				if(j == _meta_data.end()) {
					return false;
				}
			}
			return true;
		}
		
		//! Clear all meta data.
		void clear() {
			_meta_data.clear();
			_cache.clear();
		}
		
	private:			
		meta_data_container _meta_data; //!< Container for meta-data.
		meta_data_cache _cache; //!< Cache for meta-data (not ever serialized).
		
		friend class boost::serialization::access;
		template <class Archive>
		void serialize(Archive& ar, const unsigned int version) {
			ar & boost::serialization::make_nvp("meta_data", _meta_data);
		}		
	};
    
	//! Free function to put meta-data (used in command-line options processing).
	template <typename V, typename MetaData>
	void put(const std::string& k, const V& v, MetaData& md) {
		md.template put<V>(k, v);
	}	

	//! Store meta-data in an EA.
	template <typename MDType, typename HasMetaData>
	void put(const typename MDType::value_type& v, HasMetaData& hmd) {
        hmd.md().put<typename MDType::value_type>(MDType::key(), v);
	}
	
	//! Retrieve meta-data from an EA.
	template <typename MDType, typename HasMetaData>
	typename MDType::value_type get(HasMetaData& hmd) {
        return hmd.md().get<typename MDType::value_type>(MDType::key());
	}

	//! Returns true if the given meta-data exists.
	template <typename MDType, typename HasMetaData>
	bool exists(HasMetaData& hmd) {
		return hmd.md().exists(MDType::key());
	}
    
    //! Increment and set meta-data.
    template <typename MDType, typename HasMetaData>
    typename MDType::value_type next(HasMetaData& hmd) {
        if(exists<MDType>(hmd)) {
            put<MDType>(get<MDType>(hmd)+1,hmd);
        } else {
            put<MDType>(0,hmd);
        }        
        return get<MDType>(hmd);
    }
    
    //! Scale (multiply) meta-data by the given value.
    template <typename MDType, typename HasMetaData>
    void scale(const typename MDType::value_type& v, HasMetaData& hmd) {
        put<MDType>(get<MDType>(hmd)*v,hmd);
    }
    
} // ea

/* This macro defines a new meta-data item. */
#define LIBEA_MD_DECL( name, key_string, type ) \
struct name { \
typedef type value_type; \
inline static const char* key() { return key_string; } \
}
    
namespace ea {
    // ea.novelty_search.*
    LIBEA_MD_DECL(NOVELTY_THRESHOLD, "ea.novelty_search.threshold", double);
    LIBEA_MD_DECL(NOVELTY_NEIGHBORHOOD_SIZE, "ea.novelty_search.neighborhood.size", int);
    LIBEA_MD_DECL(NOVELTY_FITTEST_SIZE, "ea.novelty_search.novelty.fittest.size", int);
    
    // ea.individual.*
    LIBEA_MD_DECL(INDIVIDUAL_COUNT, "ea.individual.count", long);
    
	// ea.representation.*
	LIBEA_MD_DECL(REPRESENTATION_SIZE, "ea.representation.size", int);
	LIBEA_MD_DECL(REALSTRING_MEAN, "ea.representation.realstring.mean", double);
	LIBEA_MD_DECL(REALSTRING_VARIANCE, "ea.representation.realstring.variance", double);
	LIBEA_MD_DECL(INTSTRING_MEAN, "ea.representation.intstring.mean", int);
	LIBEA_MD_DECL(INTSTRING_VARIANCE, "ea.representation.intstring.variance", int);
	LIBEA_MD_DECL(UINTSTRING_MEAN, "ea.representation.uintstring.mean", unsigned int);
	LIBEA_MD_DECL(UINTSTRING_VARIANCE, "ea.representation.uintstring.variance", unsigned int);
	
    // ea.environment.*
    LIBEA_MD_DECL(LOCATION_COLOR, "ea.environment.location.color", int);

    // ea.fitness_function.*
    LIBEA_MD_DECL(FF_RNG_SEED, "ea.fitness_function.rng_seed", int);
    LIBEA_MD_DECL(FF_INITIAL_RNG_SEED, "ea.fitness_function.initial_rng_seed", int);
    LIBEA_MD_DECL(FF_INITIALIZATION_PERIOD, "ea.fitness_function.initialization_period", int);
    LIBEA_MD_DECL(FF_N, "ea.fitness_function.n", int);
    LIBEA_MD_DECL(NK_MODEL_N, "ea.fitness_function.nk_model.n", unsigned int);
    LIBEA_MD_DECL(NK_MODEL_K, "ea.fitness_function.nk_model.k", unsigned int);
    LIBEA_MD_DECL(NK_MODEL_BINS, "ea.fitness_function.nk_model.bins", unsigned int);
    
    // ea.landscape.*
    LIBEA_MD_DECL(LANDSCAPE_PERIOD, "ea.landscape.period", double);

	// ea.population.*
	LIBEA_MD_DECL(POPULATION_SIZE, "ea.population.size", unsigned int);
    LIBEA_MD_DECL(INITIAL_POPULATION_SIZE, "ea.population.initial_size", unsigned int);

	// ea.meta_population.*
    LIBEA_MD_DECL(META_POPULATION_SIZE, "ea.meta_population.size", unsigned int);
    LIBEA_MD_DECL(METAPOP_COMPETITION_PERIOD, "ea.meta_population.competition_period", unsigned int);

    // ea.initialization.*
    LIBEA_MD_DECL(INITIALIZATION_UNIFORM_INT_MIN, "ea.initialization.uniform_integer.min", int);
    LIBEA_MD_DECL(INITIALIZATION_UNIFORM_INT_MAX, "ea.initialization.uniform_integer.max", int);
    LIBEA_MD_DECL(INITIALIZATION_UNIFORM_REAL_MIN, "ea.initialization.uniform_real.min", double);
    LIBEA_MD_DECL(INITIALIZATION_UNIFORM_REAL_MAX, "ea.initialization.uniform_real.max", double);
    
	// ea.selection.*
	LIBEA_MD_DECL(TOURNAMENT_SELECTION_N, "ea.selection.tournament.n", int);
	LIBEA_MD_DECL(TOURNAMENT_SELECTION_K, "ea.selection.tournament.k", int);
	LIBEA_MD_DECL(ELITISM_N, "ea.selection.elitism.n", int);
	
	// ea.generational_model.*
	LIBEA_MD_DECL(STEADY_STATE_LAMBDA, "ea.generational_model.steady_state.lambda", int);
	LIBEA_MD_DECL(NUM_OFFSPRING, "ea.generational_model.num_offspring", int);
	LIBEA_MD_DECL(REPLACEMENT_RATE_P, "ea.generational_model.replacement_rate.p", double);
	
	// ea.mutation.*
	LIBEA_MD_DECL(MUTATION_GENOMIC_P, "ea.mutation.genomic.p", double);
	LIBEA_MD_DECL(MUTATION_PER_SITE_P, "ea.mutation.site.p", double);
	LIBEA_MD_DECL(MUTATION_DUPLICATION_P, "ea.mutation.duplication.p", double);
	LIBEA_MD_DECL(MUTATION_DELETION_P, "ea.mutation.deletion.p", double);
	LIBEA_MD_DECL(MUTATION_INSERTION_P, "ea.mutation.insertion.p", double);
    LIBEA_MD_DECL(MUTATION_UNIFORM_INT_MIN, "ea.mutation.uniform_integer.min", int);
    LIBEA_MD_DECL(MUTATION_UNIFORM_INT_MAX, "ea.mutation.uniform_integer.max", int);
    LIBEA_MD_DECL(MUTATION_UNIFORM_REAL_MIN, "ea.mutation.uniform_real.min", double);
    LIBEA_MD_DECL(MUTATION_UNIFORM_REAL_MAX, "ea.mutation.uniform_real.max", double);
    LIBEA_MD_DECL(MUTATION_NORMAL_REAL_MEAN, "ea.mutation.normal_real.mean", double);
    LIBEA_MD_DECL(MUTATION_NORMAL_REAL_VAR, "ea.mutation.normal_real.var", double);
    LIBEA_MD_DECL(MUTATION_CLIP_MIN, "ea.mutation.clip.min", double);
    LIBEA_MD_DECL(MUTATION_CLIP_MAX, "ea.mutation.clip.max", double);
    
	// ea.run.*
	LIBEA_MD_DECL(RUN_UPDATES, "ea.run.updates", int);
	LIBEA_MD_DECL(RUN_EPOCHS, "ea.run.epochs", int);
	LIBEA_MD_DECL(CHECKPOINT_PREFIX, "ea.run.checkpoint_prefix", std::string);
	LIBEA_MD_DECL(COMMAND_LINE, "ea.run.command_line", std::string);

    // ea.rng.*
    LIBEA_MD_DECL(RNG_SEED, "ea.rng.seed", unsigned int);
    
    // ea.statistics.*
    LIBEA_MD_DECL(RECORDING_PERIOD, "ea.statistics.recording.period", unsigned long);
    
    // ea.analysis.*
    LIBEA_MD_DECL(ANALYSIS_INPUT, "ea.analysis.input.filename", std::string);
    LIBEA_MD_DECL(ANALYSIS_OUTPUT, "ea.analysis.output.filename", std::string);
    LIBEA_MD_DECL(ANALYSIS_ROUNDS, "ea.analysis.rounds", unsigned int);
    LIBEA_MD_DECL(EPISTASIS_THRESHOLD, "ea.analysis.epistasis.threshold", double);
    
} // ea

#endif

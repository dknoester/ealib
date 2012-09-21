/* cmdline_interface.h
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
#ifndef _EA_CMDLINE_INTERFACE_H_
#define _EA_CMDLINE_INTERFACE_H_

#include <sys/resource.h>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/program_options.hpp>
#include <boost/timer.hpp>
#include <boost/regex.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/serialization/version.hpp>
#include <string>
#include <map>
#include <iostream>
#include <ea/concepts.h>
#include <ea/events.h>
#include <ea/meta_data.h>
#include <ea/checkpoint.h>
#include <ea/analysis.h>


namespace ea {
    
    /*! Datafile for run statistics.
     */
    template <typename EA>
    struct run_statistics : end_of_update_event<EA> {
        run_statistics(EA& ea) : end_of_update_event<EA>(ea) {
            _t.restart();
        }
        
        virtual ~run_statistics() {
        }
        
        virtual void operator()(EA& ea) {
            double t=_t.elapsed();
            _tacc(t);
            std::cerr << std::fixed << std::setprecision(4) << t << " ";
            std::cerr << std::fixed << std::setprecision(4) << boost::accumulators::mean(_tacc) << " ";
            
            double rss=1024.0;
#ifdef __APPLE__
            rss *= 1024.0; // bug in apple documentation; ru_maxrss is in units of bytes.
#endif
            
            rusage r;
            getrusage(RUSAGE_SELF, &r);
            std::cerr << std::fixed << std::setprecision(4) << r.ru_maxrss/rss << std::endl;
            
            
            _t.restart();
        }
        
        boost::timer _t;
        boost::accumulators::accumulator_set<double, boost::accumulators::stats<boost::accumulators::tag::mean> > _tacc;
    };

    
    //! Abstract base class allowing for a limited set of interactions with an EA.
    class ea_interface {
    public:
        //! Gather all the registered options into an options description.
        virtual void gather_options(boost::program_options::options_description& od) = 0;
        //! Run an analysis on the EA.
        virtual void analyze(boost::program_options::variables_map& vm) = 0;
        //! Continue a checkpoint.
        virtual void continue_checkpoint(boost::program_options::variables_map& vm) = 0;
        //! Run the EA.
        virtual void run(boost::program_options::variables_map& vm) = 0;
	};
    

    /*! This class is used to interface the runtime environment with an EA interface.
     */
    class registrar : ea_interface {
    public:
        //! Retrieve the registrar.
		static registrar* instance() {
			if(!_instance) {
				_instance = new registrar();
			}
			return _instance;
		}
        
        //! Gather all the registered options into an options description.
        virtual void gather_options(boost::program_options::options_description& od) {
            _ea->gather_options(od);
        }
        
        //! Run an analysis on the EA.
        virtual void analyze(boost::program_options::variables_map& vm) {
            _ea->analyze(vm);
        }
        
        //! Continue a checkpoint.
        virtual void continue_checkpoint(boost::program_options::variables_map& vm) {
            _ea->continue_checkpoint(vm);
        }
        
        //! Run the EA.
        virtual void run(boost::program_options::variables_map& vm) {
            _ea->run(vm);
        }
        
        //! Register an interface to an EA.
        void register_ea(ea_interface* ea) {
            _ea = ea;
        }
        
    protected:
        //! Constructor.
        registrar() : _ea(0) {
        }
        
		static registrar* _instance; //!< Singleton.
        ea_interface* _ea; //!< Interface to the EA.
    };
    

    template <typename T>
    struct pointer_map {
        typedef T value_type;
        typedef std::map<std::string, boost::shared_ptr<value_type> > map_type;
        map_type _m;
        
        pointer_map() { }
        virtual ~pointer_map() { }
        
        template <typename U>
        void put() {
            boost::shared_ptr<value_type> p(new U());
            _m[U::name()] = p;
        }
        
        template <typename U, typename EA>
        void put(EA& ea) {
            boost::shared_ptr<value_type> p(new U(ea));
            _m[U::name()] = p;
        }
        
        T* get(const std::string& k) {
            return _m[k].get();
        }
    };
    
    // pre-dec
    template <typename EA> class cmdline_interface;
    
    
    /* The below are unbound template functions that are to be called from the 
     appropriate "gather" method in subclasses of the cmdline_interface.  It's done
     this way to avoid additional template / macro craziness.
     */
    
    //! Add a command line option to the given EA interface.
    template <typename MDType, typename EA>
    void add_option(cmdline_interface<EA>* ci) {
        ci->_od->add_options()(MDType::key(), boost::program_options::value<std::string>());
    }
    
    //! Add an analysis tool to the tools that are registered for an EA.
    template <template <typename> class Tool, typename EA>
    void add_tool(cmdline_interface<EA>* ci) {
        typedef Tool<EA> tool_type;
        ci->_tools.put<tool_type>();
    }
    
    //! Add an event to the list of events that are registered for an EA.
    template <template <typename> class Event, typename EA>
    void add_event(cmdline_interface<EA>* ci, EA& ea) {
        typedef Event<EA> event_type;
        boost::shared_ptr<event_type> p(new event_type(ea));
        ci->_events.push_back(p);
    }
    
    
    /*! Command-line interface to an EA.
     */
    template <typename EA>
    class cmdline_interface : public ea_interface {
    public:
        typedef EA ea_type; //!< Type of the EA being used.
        typedef ea::analysis::unary_function<ea_type> tool_type; //!< Type for analysis tools.
        typedef pointer_map<tool_type> tool_registry; //!< Storage for analysis tools.
        typedef std::vector<boost::shared_ptr<ea::event> > event_list; //!< Storage for events.
        
        //! Constructor.
        cmdline_interface() {
            registrar::instance()->register_ea(this);
        }
        
        //! Gather all the registered options into an options description.
        void gather_options(boost::program_options::options_description& od) {
            _od = &od;
            gather_options();
        }
        
        //! Gather the options supported by this EA.
        virtual void gather_options() = 0;
        
        //! Gather the analysis tools supported by this EA.
        virtual void gather_tools() = 0;
        
        //! Gather the events that occur during a trial of this EA.
        virtual void gather_events(EA& ea) = 0;

        //! Analyze an EA instance.
		virtual void analyze(boost::program_options::variables_map& vm) {
			ea_type ea;
            load_if(vm, ea);
            apply(vm, ea);
            ea.initialize();
            gather_tools();
            (*_tools.get(vm["analyze"].as<std::string>()))(ea);
        }
        
        //! Continue a previously-checkpointed EA.
		virtual void continue_checkpoint(boost::program_options::variables_map& vm) {
            ea_type ea;
            load(vm, ea);
            
            // conditionally apply command-line and/or file parameters:
            if(vm.count("override")) {
                apply(vm, ea);
            }
            
            // conditionally reset all fitnesses
            if(vm.count("reset")) {
                ea.reset();
            }
            
            ea.initialize();
            gather_events(ea);
            execute(ea);
        }        
        
		//! Run the EA.
		virtual void run(boost::program_options::variables_map& vm) {
			ea_type ea;
            apply(vm, ea);
            
            if(exists<RNG_SEED>(ea)) {
                ea.rng().reset(get<RNG_SEED>(ea));
            }
            
            ea.initialize();
            gather_events(ea);
            
            if(vm.count("with-time")) {
                add_event<run_statistics>(this,ea);
            }
            
            ea.generate_initial_population();
            execute(ea);
        }
        
    protected:

        //! Apply any command line options to the EA.
        void apply(boost::program_options::variables_map& vm, EA& ea) {
            namespace po = boost::program_options;
            po::notify(vm);
            std::cerr << std::endl << "Active configuration options:" << std::endl;
            for(po::variables_map::iterator i=vm.begin(); i!=vm.end(); ++i) {
                const std::string& k=i->first;
                const std::string& v=i->second.as<std::string>();
                std::cerr << "\t" << k << "=" << v << std::endl;
                put(k, v, ea.md());
            }
            std::cerr << std::endl;
        }

        //! Returns true if a checkpoint file should be loaded.
        bool has_checkpoint(boost::program_options::variables_map& vm) {
            return (vm.count("checkpoint") > 0);
        }

        //! Load the EA from the checkpoint file, if present.
        void load_if(boost::program_options::variables_map& vm, ea_type& ea) {
            if(has_checkpoint(vm)) {
                load(vm, ea);
            }
        }
        
        //! Load the EA from the checkpoint file.
        void load(boost::program_options::variables_map& vm, ea_type& ea) {
            if(!has_checkpoint(vm)) {
                throw fatal_error_exception("required checkpoint file not found.");
            }
            std::string cpfile(vm["checkpoint"].as<std::string>());
            std::ifstream ifs(cpfile.c_str());
            if(!ifs.good()) {
                throw file_io_exception("could not open " + cpfile + " for reading.");
            }
            std::cerr << "loading " << cpfile << "... ";
            
            // is this a gzipped file?  test by checking file extension...
            static const boost::regex e(".*\\.gz$");
            if(boost::regex_match(cpfile, e)) {
                namespace bio = boost::iostreams;
                bio::filtering_stream<bio::input> f;
                f.push(bio::gzip_decompressor());
                f.push(ifs);
                checkpoint_load(ea, f);
            } else {
                checkpoint_load(ea, ifs);
            }
            std::cerr << "done." << std::endl;
        }
        
        //! Execute the EA for the configured number of epochs.
        void execute(ea_type& ea) {
			// and run it!
			for(int i=0; i<get<RUN_EPOCHS>(ea); ++i) {
                ea.advance_epoch(get<RUN_UPDATES>(ea));
                if(get<CHECKPOINT_ON>(ea)) {
                    checkpoint(ea);
                }
			}
		}
        
        // This grants the following templated functions access to the internals
        // of the cmdline_interface... a little hackish, but it avoids macro/template
        // craziness.
        template <typename T, typename U> friend void add_option(cmdline_interface<U>*);
        template <template <typename> class T, typename U> friend void add_tool(cmdline_interface<U>* ci);
        template <template <typename> class T, typename U> friend void add_event(cmdline_interface<U>* ci, U& u);

        boost::program_options::options_description* _od; //!< Options that are configured for this EA.
        tool_registry _tools; //!< Registry for EA analysis tools.
        event_list _events; //!< List of all the events attached to an EA.
    };

} // ea


/*! Declare an instance of an evolutionary algorithm.
 
 This is where we adjust serialization versions, if needed...
 */
#define LIBEA_CMDLINE_INSTANCE( ea_type, cmdline_type ) \
cmdline_type<ea_type> cmdline_type##_instance;
//BOOST_CLASS_VERSION(ea_type::individual_type, 2) 
//BOOST_CLASS_VERSION(ea_type::generational_model_type, 1)

#endif

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

#include <boost/program_options.hpp>
#include <boost/regex.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/serialization/version.hpp>
#include <string>
#include <map>
#include <iostream>

#include <ea/algorithm.h>
#include <ea/concepts.h>
#include <ea/events.h>
#include <ea/meta_data.h>
#include <ea/population.h>
#include <ea/analysis/tool.h>
#include <ea/lifecycle.h>
#include <ea/datafiles/runtime.h>
#include <ea/rng.h>

namespace ealib {

    LIBEA_MD_DECL(COMMAND_LINE, "ea.run.command_line", std::string);

    //! Abstract base class allowing for a limited set of interactions with an EA.
    class ea_interface {
    public:
        //! Execute an EA based on the given command-line parameters.
        virtual void exec(int argc, char* argv[]) = 0;
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
        
        //! Execute an EA based on the given command-line parameters.
        virtual void exec(int argc, char* argv[]) {
            _ea->exec(argc, argv);
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
    
    // pre-dec
    template <typename EA> class cmdline_interface;
    
    /* The below are unbound template functions that are to be called from the 
     appropriate "gather" method in subclasses of the cmdline_interface.  It's done
     this way to avoid additional template / macro craziness.
     */
    
    //! Add a command line option to the given EA interface.
    template <typename MDType, typename EA>
    void add_option(cmdline_interface<EA>* ci) {
        ci->_ea_options.add_options()(MDType::key(), boost::program_options::value<std::string>());
    }
    
    //! Add an analysis tool to the tools that are registered for an EA.
    template <template <typename> class Tool, typename EA>
    void add_tool(cmdline_interface<EA>* ci) {
        typedef Tool<EA> tool_type;
        boost::shared_ptr<tool_type> p(new tool_type());
        ci->_tools[tool_type::name()] = p;
    }
    
    
    /*! This selector is used with the command-line interface to automatically 
     register the command line interface.
     */
    struct do_not_registerS { };
    
    /*! Command-line interface to an EA.
     */
    template <typename EA>
    class cmdline_interface : public ea_interface {
    public:
        typedef EA ea_type; //!< Type of the EA being used.
        typedef ealib::analysis::unary_function<ea_type> tool_type; //!< Type for analysis tools.
        typedef boost::shared_ptr<tool_type> tool_ptr_type; //!< Pointer type for analysis tools.
        typedef std::map<std::string, tool_ptr_type> tool_registry; //!< Storage for analysis tools.
        
        //! Registering constructor.
        cmdline_interface() : ea_interface(), _ea_options("Configuration file and command-line options") {
            registrar::instance()->register_ea(this);
        }

        //! Non-registering constructor.
        cmdline_interface(do_not_registerS) : ea_interface(), _ea_options("Configuration file and command-line options") {
        }
        
        //! Gather the options supported by this EA.
        virtual void gather_options() { }
        
        //! Gather the analysis tools supported by this EA.
        virtual void gather_tools() { }
        
        //! Gather the events that occur during a trial of this EA.
        virtual void gather_events(EA& ea) { }
        
        //! Execute an EA based on the given command-line parameters.
        virtual void exec(int argc, char* argv[]) {
            gather_options();
            parse_all(argc, argv);
            
            ea_type ea;
            
            if(_vm.count("analyze")) {
                analyze(ea);
            } else if(_vm.count("checkpoint")) {
                continue_checkpoint(ea);
            } else {
                run(ea);
            }
        }
        
        //! Parse config file options.
        void parse_config_file(const std::string& filename) {
            namespace po = boost::program_options;
            using namespace std;

            ifstream ifs(filename.c_str());
            if(ifs.good()) {
                po::parsed_options opt=po::parse_config_file(ifs, _ea_options, true);
                vector<string> unrec = po::collect_unrecognized(opt.options, po::exclude_positional);
                if(!unrec.empty()) {
                    ostringstream msg;
                    msg << "Unrecognized options were found in: " << filename << ":" << endl;
                    for(std::size_t i=0; i<unrec.size(); ++i) {
                        msg << "\t" << unrec[i] << endl;
                    }
                    msg << "Exiting..." << endl;
                    throw ealib::ealib_exception(msg.str());
                }
                po::store(opt, _vm);
                po::notify(_vm);
                ifs.close();
            } else {
                throw ealib::file_io_exception("Could not open config file: " + filename);
            }
        }

        //! Parse command-line and potentially config file options.
        void parse_all(int argc, char* argv[]) {
            namespace po = boost::program_options;
            using namespace std;
            
            po::options_description cmdline_only_options("Command-line only options");
            cmdline_only_options.add_options()
            ("help,h", "produce this help message")
            ("config,c", po::value<string>(), "ealib configuration file")
            ("checkpoint,l", po::value<string>(), "load a checkpoint file")
            ("override", "override checkpoint options")
            ("reset", "reset all fitness values prior to continuing a checkpoint")
            ("analyze", po::value<string>(), "analyze the results of this EA")
            ("verbose", "output configuration options and per-update time and memory usage");

            po::options_description all_options;
            all_options.add(cmdline_only_options).add(_ea_options);
            
            po::store(po::parse_command_line(argc, argv, all_options), _vm);
            po::notify(_vm);
            
            if(_vm.count("config")) {
                parse_config_file(_vm["config"].template as<string>());
            }
            
            if(_vm.count("help")) {
                ostringstream msg;
                msg << "Usage: " << argv[0] << " [-c config_file] [--verbose] [-l checkpoint] [--override] [--analyze] [--option_name value...]" << endl;
                msg << all_options << endl;
                throw ealib::ealib_exception(msg.str());
            }
            
            po::notify(_vm);
        }

        //! Analyze an EA instance.
		void analyze(ea_type& ea) {
            load_if(ea);
            apply(ea);
            ea.initialize();
            gather_tools();
            
            std::string toolname = _vm["analyze"].template as<std::string>();
            typename tool_registry::iterator i = _tools.find(toolname);
            if(i == _tools.end()) {
                throw bad_argument_exception("Could not find analysis tool: " + toolname);
            }
            
            i->second->initialize(ea);
            (*i->second)(ea);
        }
        
        //! Continue a previously-checkpointed EA.
		void continue_checkpoint(ea_type& ea) {
            load(ea);
            
            // conditionally apply command-line and/or file parameters:
            if(_vm.count("override")) {
                apply(ea);
            }
            
            // conditionally reset all fitnesses
            if(_vm.count("reset")) {
                ea.reset();
            }
            
            ea.initialize();
            gather_events(ea);
            if(_vm.count("verbose")) {
                add_event<datafiles::runtime>(ea);
            }
            lifecycle::advance_all(ea);
        }
        
        //! Initialize an EA.
        void initialize_ea(ea_type& ea) {
            apply(ea);
            
            if(exists<RNG_SEED>(ea)) {
                ea.rng().reset(get<RNG_SEED>(ea));
            }
            
            ea.initialize();
            gather_events(ea);
            if(_vm.count("verbose")) {
                add_event<datafiles::runtime>(ea);
            }
            ea.initial_population();
        }

        //! Continue an already initialized EA for another epoch.
        void continue_ea(ea_type& ea) {
            lifecycle::advance_all(ea);
        }

		//! Run the EA.
		void run(ea_type& ea) {
            initialize_ea(ea);
            continue_ea(ea);
        }
        
    protected:

        //! Apply meta-data to a single population.
        void apply(const std::string& k, const std::string& v, EA& ea, bool verbose, singlePopulationS) {
            if(verbose) {
                std::cerr << "\t" << k << "=" << v << std::endl;
            }
            put(k, v, ea.md());
        }
        
        //! Apply meta-data to multiple populations.
        void apply(const std::string& k, const std::string& v, EA& ea, bool verbose, multiPopulationS) {
            if(verbose) {
                std::cerr << "\t" << k << "=" << v << " (+subpopulations)" << std::endl;
            }
            put(k, v, ea.md());
            for(typename EA::iterator i=ea.begin(); i!=ea.end(); ++i) {
                put(k,v,i->md());
            }
        }

        //! Apply any command line options to the EA.
        void apply(EA& ea) {
            namespace po = boost::program_options;
            bool verbose = (_vm.count("verbose") > 0);

            if(verbose) {
                std::cerr << std::endl << "Active configuration options:" << std::endl;
            }
            
            for(po::variables_map::iterator i=_vm.begin(); i!=_vm.end(); ++i) {
                const std::string& k=i->first;
                const std::string& v=i->second.as<std::string>();
                apply(k, v, ea, verbose, typename EA::population_structure_tag());
            }
            
            if(verbose) {
                std::cerr << std::endl;
            }
        }
        
        //! Returns true if a checkpoint file should be loaded.
        bool has_checkpoint() {
            return (_vm.count("checkpoint") > 0);
        }

        //! Load the EA from the checkpoint file, if present.
        void load_if(ea_type& ea) {
            if(has_checkpoint()) {
                load(ea);
            }
        }
        
        //! Load the EA from the checkpoint file.
        void load(ea_type& ea) {
            if(!has_checkpoint()) {
                throw fatal_error_exception("required checkpoint file not found.");
            }
            std::string cpfile(_vm["checkpoint"].template as<std::string>());
            lifecycle::load_checkpoint(cpfile, ea);
        }
        
        // This grants the following templated functions access to the internals
        // of the cmdline_interface... a little hackish, but it avoids macro/template
        // craziness.
        template <typename T, typename U> friend void add_option(cmdline_interface<U>*);
        template <template <typename> class T, typename U> friend void add_tool(cmdline_interface<U>* ci);

        boost::program_options::options_description _ea_options; //!< Options that are configured for this EA.
        boost::program_options::variables_map _vm; //!< Variables (loaded from options).
        tool_registry _tools; //!< Registry for EA analysis tools.
    };

} // ea


/*! Declare an instance of an evolutionary algorithm, and connect it to the registrar
 for command-line access.
 */
#define LIBEA_CMDLINE_INSTANCE( ea_type, cmdline_type ) \
cmdline_type<ea_type> cmdline_type##_instance;
//BOOST_CLASS_VERSION(ea_type::individual_type, 2) 
//BOOST_CLASS_VERSION(ea_type::generational_model_type, 1)

#endif

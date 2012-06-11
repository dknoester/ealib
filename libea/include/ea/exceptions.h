/* exceptions.h
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
#ifndef _EA_EXCEPTIONS_H_
#define _EA_EXCEPTIONS_H_

#include <iostream>
#include <string>


namespace ea {

	//! EAlib exception base class.
	struct ealib_exception {
		ealib_exception() { }
		ealib_exception(const std::string& m) : msg(m) {	}
        std::string msg;
	};
	
	//! Thrown if a required property is not found.
	struct property_not_found_exception : ealib_exception {
		property_not_found_exception(const std::string& m) : ealib_exception("property not found: " + m) {
		}	
	};
	
	//! Thrown if a fatal error occurs.
	struct fatal_error_exception : ealib_exception { 
		fatal_error_exception(const std::string& m) : ealib_exception("fatal error: " + m) {
		}
	};
	
	//! Thrown if a method is not yet implemented.
	struct not_yet_implemented_exception : ealib_exception {
		not_yet_implemented_exception(const std::string& m) : ealib_exception("not yet implemented") {
		}
	};

	//! Thrown when meta data is retrieved prior to being initialized.
	struct uninitialized_meta_data_exception : ealib_exception {
		uninitialized_meta_data_exception(const std::string& m) : ealib_exception("uninitialized meta data: " + m) {
		}
	};
	
	//! Thrown when a component of ealib is used incorrectly.
	struct usage_error_exception : ealib_exception {
		usage_error_exception(const std::string& m) : ealib_exception("usage error: " + m) {
		}
	};
    
    //! Thrown when file io operations fail.
    struct file_io_exception : ealib_exception {
        file_io_exception(const std::string& m) : ealib_exception("file io error: " + m) {
        }
    };

    //! Thrown when there is a problem with an parameter.
    struct bad_argument_exception : ealib_exception {
        bad_argument_exception(const std::string& m) : ealib_exception("bad argument: " + m) {
        }
    };

    //! Check an argument for some condition, and throw an exception if the condition failed.
    inline void check_argument(bool cond, const std::string& m) {
        if(!cond) {
            throw bad_argument_exception(m);
        }
    }
    
} // ea

#endif

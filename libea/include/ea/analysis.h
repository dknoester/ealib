/* analysis.h 
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

#ifndef _EA_ANALYSIS_H_
#define _EA_ANALYSIS_H_

#include <ea/meta_data.h>

// ea.analysis.*
LIBEA_MD_DECL(ANALYSIS_INPUT, "ea.analysis.input.filename", std::string);
LIBEA_MD_DECL(ANALYSIS_OUTPUT, "ea.analysis.output.filename", std::string);


//! Convenience macro for analysis tools.
#define LIBEA_ANALYSIS_TOOL(toolname) \
template <typename EA> \
struct toolname : public ealib::analysis_tool<EA> { \
static const char* name() { return #toolname; } \
virtual ~toolname() { }; \
void operator()(EA& ea); }; \
template <typename EA> void toolname<EA>::operator()(EA& ea)


namespace ealib {
    
    /*! Abstract base class for analysis tools.
     */
    template <typename EA>
    struct analysis_tool {
        //! Destructor.
        virtual ~analysis_tool() { }
        
        //! Run this analysis tool on ea.
        virtual void operator()(EA& ea) = 0;
    };
    
} // ealib

#endif

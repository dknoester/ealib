/* datafile.h
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
#ifndef _EA_DATAFILE_H_
#define _EA_DATAFILE_H_

#include <boost/algorithm/string/trim_all.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/regex.hpp>

#include <iomanip>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <bitset>

#include <ea/algorithm.h>
#include <ea/exceptions.h>

namespace ealib {
    
    /*! Provides convenient facilities for writing datafiles.
     */
    class datafile {
    public:
        typedef std::vector<std::string> string_vector;
        
        datafile(const std::string& fname) {
            open(fname.c_str());
        }
        
        datafile(const std::string& prefix, int i, const std::string& suffix) {
            std::ostringstream fname;
            fname << prefix << "-" << i << suffix;
            open(fname.str().c_str());
        }
        
        virtual ~datafile() {
            _out.close();
        }
        
        operator std::ostream&() {
            return _out;
        }
        
        datafile& add_field(const std::string& fieldname, const std::string& desc="") {
            _header.push_back(fieldname);
            if(desc.size() > 0) {
                comment(fieldname + ": " + desc);
            }
            return *this;
        }
        
        template <typename ForwardIterator>
        datafile& write(ForwardIterator f, ForwardIterator l) {
            for( ; f!=l; ++f) {
                write(*f).endl();
            }
            return *this;
        }
        
        template <typename ForwardIterator>
        datafile& write_all(ForwardIterator f, ForwardIterator l) {
            for( ; f!=l; ++f) {
                write(*f);
            }
            return *this;
        }

        datafile& write(float v) {                
            _row << std::fixed << std::setprecision(4) << v << " ";
            return *this;
        }

        datafile& write(double v) {                
            _row << std::fixed << std::setprecision(4) << v << " ";
            return *this;
        }
        
        datafile& write(int v) {
            _row << v << " ";
            return *this;
        }
        
        datafile& write(unsigned int v) {
            _row << v << " ";
            return *this;
        }

        datafile& write(long v) {
            _row << v << " ";
            return *this;
        }

        datafile& write(long unsigned int v) {
            _row << v << " ";
            return *this;
        }

        datafile& write(const std::string& s) {
            _row << s << " ";
            return *this;
        }
        
        datafile& writeq(const std::string& s) {
            _row << "\"" << s << "\" ";
            return *this;
        }
        
        datafile& writeb64(std::size_t v) {
            _row << std::bitset<64>(v) << " ";
            return *this;
        }
        
        datafile& writeb32(std::size_t v) {
            _row << std::bitset<32>(v) << " ";
            return *this;
        }
        
        datafile& writeNA() {
            _row << "NA" << " ";
            return *this;
        }
        
        datafile& comment(const std::string& s) {
            _comments.push_back(s);
            return *this;
        }
        
        datafile& endl() {
            write_comments();
            write_header();
            std::string r=_row.str();
            if(r.size() > 0) {
                r.erase(r.size()-1); // we always have a single trailing whitespace.
                _out << r;
            }
            _out << std::endl;
            _row.str("");
            _row.clear();
            return *this;
        }
        
    protected:
        void write_header() {
            if(_header.size() > 0) {
                _out << algorithm::vcat(_header.begin(), _header.end(), " ") << std::endl;
                _header.clear();
            }
        }
        
        void write_comments() {
            if(_comments.size() > 0) {
                _out << "# " << algorithm::vcat(_comments.begin(), _comments.end(), "\n# ") << std::endl;
                _comments.clear();
            }
        }
        
        void open(const char* f) {
            _out.open(f);
            if(!_out.good()) {
                throw file_io_exception(std::string("could not open file: ") + f);
            }
        }                                  
        
        std::ofstream _out;
        string_vector _header;
        string_vector _comments;
        std::ostringstream _row;
    };
    

    /*! First cut at an input datafile reader.
     */
    class idatafile {
    public:
        typedef std::vector<std::string> row_type;
        typedef std::vector<row_type> matrix_type;
        typedef matrix_type::iterator iterator;
        typedef matrix_type::const_iterator const_iterator;
        
        idatafile(const std::string& filename, const std::string& sep=",") {
            using namespace boost;
            
            std::ifstream infile(filename.c_str());
        
            if(!infile.is_open()) {
                throw file_io_exception("datafile.h::initialize: could not open " + filename);
            }
        
            static const regex comment("^#.*");
            bool header=false;
            std::string line;
            
            while(getline(infile, line)) {
                // remove leading & trailing whitespace:
                trim_all(line);
                // only consider lines with length > 0:
                if(line.empty()) {
                    continue;
                }
                // skip all comments:
                if(regex_match(line, comment)) {
                    continue;
                }

                // the first non-comment line is assumed to be the header:
                if(!header) {
                    split(_colnames, line, is_any_of(sep));
                    header = true;
                    continue;
                }
            
                // split all remaining lines into fields, add them to the string matrix:
                row_type row;
                split(row, line, is_any_of(sep));
                _data.push_back(row);
            }
        }
        
        //! Retrieve an iterator to the beginning of the data matrix.
        iterator begin() { return _data.begin(); }
      
        //! Retrieve an iterator to the beginning of the data matrix (const-qualified).
        const_iterator begin() const { return _data.begin(); }
        
        //! Retrieve an iterator to the end of the data matrix.
        iterator end() { return _data.end(); }
        
        //! Retrieve an iterator to the end of the data matrix (const-qualified).
        const_iterator end() const { return _data.end(); }
        
        /*! Convenience method to translate this datafile to a different format.
         
         The value_type of Container must provide a constructor that can accept
         a row_type.  Container must support push_back.
         */
        template <typename Container, typename EA>
        void translate(Container& c, EA& ea) {
            for(iterator i=begin(); i!=end(); ++i) {
                c.push_back(typename Container::value_type(*i,ea));
            }
        }
        
    protected:
        row_type _colnames;
        matrix_type _data;
    };

} // ea

#endif

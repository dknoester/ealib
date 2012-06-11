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

#include <iomanip>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <bitset>
#include <ea/algorithm.h>
#include <ea/exceptions.h>

namespace ea {
    
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

        datafile& write(double v) {                
            _row << std::fixed << std::setprecision(4) << v << " ";
            return *this;
        }
        
        datafile& write(std::size_t v) {
            _row << v << " ";
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
    
    
//    class idatafile {
//    public:
//
//        idatafile(const std::string& fname) {        
//        }
//        
//        if(!has_checkpoint(vm)) {
//            throw fatal_error_exception("required checkpoint file not found.");
//        }
//        std::string cpfile(vm["checkpoint"].as<std::string>());
//        std::ifstream ifs(cpfile.c_str());
//        std::cerr << "loading " << cpfile << "... ";
//        
//        // is this a gzipped file?  test by checking file extension...
//        static const boost::regex e(".*\\.gz$");
//        if(boost::regex_match(cpfile, e)) {
//            namespace bio = boost::iostreams;
//            bio::filtering_stream<bio::input> f;
//            f.push(bio::gzip_decompressor());
//            f.push(ifs);
//            checkpoint_load(ea, f);
//        } else {
//            checkpoint_load(ea, ifs);
//        }
//        std::cerr << "done." << std::endl;
//
//        template <typename EA>
//        void checkpoint_load(EA& ea, std::istream& in) {
//            boost::archive::xml_iarchive ia(in);
//            ia >> BOOST_SERIALIZATION_NVP(ea);
//        }
//
//    };
    
} // ea

#endif

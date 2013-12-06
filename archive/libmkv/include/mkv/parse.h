/* parse.h
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
#ifndef _MKV_PARSE_H_
#define _MKV_PARSE_H_


#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/fusion/adapted/boost_tuple.hpp>
#include <boost/fusion/include/boost_tuple.hpp>
#include <string>
#include <stdexcept>
#include <vector>

#include <mkv/markov_network.h>
#include <mkv/deep_markov_network.h>

namespace mkv {
    namespace detail {
        
        namespace qi = boost::spirit::qi;
        namespace ascii = boost::spirit::ascii;
             
        /*! Defines the grammar to parse a description object for a markov_network.
         */
        template <typename Iterator>
        struct desc_parser : qi::grammar<Iterator, markov_network::desc_type(), ascii::space_type> {
            
            desc_parser() : desc_parser::base_type(start) {
                using qi::int_;
                
                start %= "(" >> int_ >> "," >> int_ >> "," >> int_ >> ")";
            }
            
            qi::rule<Iterator, markov_network::desc_type(), ascii::space_type> start;
        };
        
        /*! Defines the grammar to parse a description object for a deep markov network.
         */
        template <typename Iterator>
        struct deep_desc_parser : qi::grammar<Iterator, deep_markov_network::desc_type(), ascii::space_type> {
            
            deep_desc_parser() : deep_desc_parser::base_type(start) {
                using qi::int_;
                
                layer %= "(" >> int_ >> "," >> int_ >> "," >> int_ >> ")";
                start %= layer % ",";
            }
            
            qi::rule<Iterator, markov_network::desc_type(), ascii::space_type> layer;
            qi::rule<Iterator, deep_markov_network::desc_type(), ascii::space_type> start;
        };
        
    } // detail
        
    
    /*! Parse a deep_markov_network description from a string.
     */
    template <typename T>
    void parse_desc(const T& t, deep_markov_network::desc_type& desc) {
        using boost::spirit::ascii::space;
        typedef std::string::const_iterator iterator_type;
        
        detail::deep_desc_parser<iterator_type> parser;
        desc.clear();
        
        typename T::const_iterator iter = t.begin();
        typename T::const_iterator end = t.end();
        if(!phrase_parse(iter, end, parser, space, desc)) {
            throw std::invalid_argument("could not parse deep_markov_network description");
        }
    }
    
    
    /*! Parse a markov_network description from a string.
     */
    template <typename T>
    void parse_desc(const T& t, markov_network::desc_type& desc) {
        using boost::spirit::ascii::space;
        typedef std::string::const_iterator iterator_type;
        
        detail::desc_parser<iterator_type> parser;
        
        typename T::const_iterator iter = t.begin();
        typename T::const_iterator end = t.end();
        if(!phrase_parse(iter, end, parser, space, desc)) {
            throw std::invalid_argument("could not parse markov_network description");
        }
    }
    
} // mkv

#endif

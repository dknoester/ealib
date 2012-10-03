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

#include <stdexcept>
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include <mkv/markov_network.h>
#include <mkv/detail/logic_gate.h>
#include <mkv/detail/markov_gate.h>
#include <mkv/detail/adaptive_gate.h>


BOOST_FUSION_ADAPT_STRUCT(
                          mkv::markov_network,
                          (mkv::addr_list_type, inputs)
                          (mkv::addr_list_type, outputs)
                          (mkv::addr_list_type, table)
                          )

namespace mkv {
    
    namespace detail {
        
        namespace qi = boost::spirit::qi;

        /*! Parse a Markov network.  This class defines the grammar from which we'll
         build Markov networks.
         */
        template <typename Iterator>
        struct markov_network_parser : qi::grammar<Iterator, markov_network()> {
            markov_network_parser() : logic_gate_parser::base_type(start) {
                using qi::uint_;
                using qi::lit;
                
//                addr %= lit("(") >> uint_ % ',' >> lit(")");

                logic %= lit(41) >> lit(214);
                markov %= lit(42) >> lit(213);
                adaptive %= lit(43) >> lit(212);
                
                start %= +(logic | markov | adaptive);
            }
            
//            qi::rule<Iterator, mkv::addr_list_type()> addr;
            qi::rule<Iterator, logic_gate()> logic;
            qi::rule<Iterator, markov_gate()> markov;
            qi::rule<Iterator, adaptive_gate()> adaptive;
            qi::rule<Iterator, markov_network()> start;
        };
    }
    
    
    /*! Parse a Markov network from the range [f,l).
     */
    template <typename Iterator>
    void parse(Iterator f, Iterator l, markov_network& net) {
        using namespace detail;        
        typedef markov_network_parser<Iterator> parser;
        
        parser p;            
        if(!phrase_parse(f, l, p, net)) {
            throw std::domain_error("mkv::parse: error during parse");
        }
    }
    
}

#endif

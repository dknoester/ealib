/* lsystem.h
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
#ifndef _EA_LSYS_LSYSTEM_H_
#define _EA_LSYS_LSYSTEM_H_

#include <set>
#include <map>
#include <vector>

namespace ealib {
    namespace lsys {
        
        /* Lindenmeyer system (L-system) class.
         
         This class implements a context-free deterministic L-system, which means
         that productions do not depend on neighboring symbols and each symbol can
         appear as the successor in at most one production rule.  We define an
         L-system as:
         
         G = (V, \omega, P)
         
         where V is the alphabet (set of all possible symbols), \omega is the axiom
         (initial state of the system), and P is the set of production rules.
         Symbols a \in V which are \notin P are assumed to be constants.
         
         The production rules are a map of predecessor symbol -> successor strings.
         */
        template <typename Symbol>
        class lsystem {
        public:
            typedef Symbol symbol_type;
            typedef std::set<symbol_type> alphabet_type;
            
            //! Convenience wrapper that makes it easy to assemble Lsystem strings.
            template <typename T, typename U>
            struct string_wrapper : T {
                typedef T parent;
                string_wrapper& add(const U& u) {
                    parent::push_back(u);
                    return *this;
                }
            };
            
            typedef string_wrapper<std::vector<symbol_type>, symbol_type> string_type;
            typedef std::map<symbol_type, string_type> production_type;
            
            //! Default constructor.
            lsystem() {
            }
            
            //! Constructor.
            lsystem(const alphabet_type& v, const string_type& o, const production_type& p)
            : _V(v), _omega(o), _P(p) {
            }
            
            //! Execute the lsystem on the symbols in sin, producing sout.
            void exec(string_type& sout, const string_type& sin) const {
                sout.clear();
                for(typename string_type::const_iterator i=sin.begin(); i!=sin.end(); ++i) {
                    typename production_type::const_iterator j=_P.find(*i);
                    if(j != _P.end()) {
                        sout.insert(sout.end(), j->second.begin(), j->second.end());
                    } else {
                        sout.insert(sout.end(), *i);
                    }
                }
            }
            
            //! Update the lsystem n times from the axiom, and return the final string.
            string_type exec_n(std::size_t n) const {
                string_type sout;
                if(n >= 1) {
                    string_type so, si;
                    exec(so, _omega);
                    for( --n; n>0; --n) {
                        std::swap(so,si);
                        exec(so, si);
                    }
                    std::swap(so, sout);
                }
                return sout;
            }
            
            //! Build a string from a container.
            template <typename Container>
            string_type split(Container& c) const {
                string_type s;
                for(typename Container::iterator i=c.begin(); i!=c.end(); ++i) {
                    s.push_back(static_cast<symbol_type>(*i));
                }
                return s;
            }
            
            //! Build a string from a c-style array.
            string_type splitc(const symbol_type* c) const {
                string_type s;
                while(*c != '\0') {
                    s.push_back(static_cast<char>(*c));
                    ++c;
                }
                return s;
            }
            
            //! Build a string.
            string_type string(const symbol_type& s) const {
                string_type st;
                st.push_back(s);
                return st;
            }
            
            //! Set the initial state (axiom).
            lsystem& axiom(const string_type& s) {
                _omega = s;
                return *this;
            }
            
            //! Add a variable to this lsystem.
            lsystem& symbol(const symbol_type& s) {
                _V.insert(s);
                return *this;
            }
            
            //! Add a rule to this lsystem.
            lsystem& rule(const symbol_type& p, const string_type& s) {
                _P[p] = s;
                return *this;
            }
            
            
        protected:
            alphabet_type _V; //!< Set of all symbols this lsystem understands.
            string_type _omega; //!< Initial state (aka, axiom) of this lsystem.
            production_type _P; //!< Production rules.
        };
        
    } // lsys
} // ealib

#endif

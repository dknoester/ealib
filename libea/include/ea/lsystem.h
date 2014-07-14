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
#ifndef _EA_LSYSTEM_H_
#define _EA_LSYSTEM_H_

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <stack>

namespace ealib {
    namespace bnu = boost::numeric::ublas;

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
    
    

    
    /*! Simple 2D echoing coordinate system.
     */
    struct echo_grid2 {
        template <typename Point>
        void line(const Point& p1, const Point& p2) {
            std::cout << "l: (" << p1(0) << "," << p1(1) << ") -> ("
            << p2(0) << "," << p2(1) << ")" << std::endl;
        }
        template <typename Point>
        void point(const Point& p) {
            std::cout << "p: (" << p(0) << "," << p(1) << ")" << std::endl;
        }
    };

    /*! Simple 2D coordinate system that prints out Python plotting commands.
     */
    struct python_grid2 {
        python_grid2(const std::string& filename) {
            _out.open(filename.c_str());
            _out << "import pylab as pl" << std::endl;
            _out << "import matplotlib as mp" << std::endl << std::endl;
        }
        
        ~python_grid2() {
            _out << "pl.show()" << std::endl;
            _out.close();
        }
        
        template <typename Point>
        void line(const Point& p1, const Point& p2) {
            _out << "pl.plot([" << p1(0) << "," << p2(0) << "], [" << p1(1) << "," << p2(1) << "], c=\"red\")" << std::endl;
        }
        template <typename Point>
        void point(const Point& p) {
            _out << "pl.plot([" << p(0) << "], [" << p(1) << "], 'o', markersize=3, c=\"red\")" << std::endl;
        }
        
        std::ofstream _out;
    };
    
    
    /*! Context for 2D turtles.
     */
    struct turtle_context2 {
        typedef bnu::vector<double> vector_type;
        typedef vector_type point_type;
        typedef bnu::matrix<double> rotation_matrix_type;
        
        //! Default constructor.
        turtle_context2() : _mag(1.0), _scale(1.0) {
        }
        
        //! Sets the angle for both cw (-d) and ccw (+d) rotations.
        turtle_context2& angle(double d) {
            double theta=d*boost::math::constants::pi<double>() / 180.0;
            rotation_matrix(_Rccw, theta);
            rotation_matrix(_Rcw, -theta);
            return *this;
        }
        
        //! Sets the initial origin of this context.
        turtle_context2& origin(double x, double y) {
            _p.resize(2);
            _p(0) = x; _p(1) = y;
            return *this;
        }
        
        //! Sets the initial heading of this context.
        turtle_context2& heading(double x, double y) {
            _h.resize(2);
            _h(0) = x; _h(1) = y;
            _h = _h / bnu::norm_2(_h);
            return *this;
        }
        
        //! Sets the magnitude of the distance traveled during a single step.
        turtle_context2& step_magnitude(double d) {
            _mag = d;
            return *this;
        }
        
        //! Set the depth scaling factor.
        turtle_context2& scaling_factor(double d) {
            _scale = d;
            return *this;
        }
        
        /*! Move x steps from the current position in the direction of the
         current heading, scaled by the given depth d.
         */
        void step(double x, double d=1.0) {
            _p += x * _mag * pow(_scale,d) * _h;
        }
        
        //! Returns the current position of this context.
        point_type point() {
            return _p;
        }
        
        //! Sets the given matrix to rotate by theta radians.
        void rotation_matrix(rotation_matrix_type& R, double theta) {
            R.resize(2,2);
            R(0,0) = R(1,1) = cos(theta);
            R(0,1) = -sin(theta);
            R(1,0) = -R(0,1);
        }
        
        //! Rotates this context by R.
        void rotate(const rotation_matrix_type& R) {
            _h = bnu::prod(R, _h);
        }
        
        //! Rotate ccw n times.
        void rotate_ccw(std::size_t n=1) {
            for( ; n>0; --n) {
                rotate(_Rccw);
            }
        }
        
        //! Rotate cw n times.
        void rotate_cw(std::size_t n=1) {
            for( ; n>0; --n) {
                rotate(_Rcw);
            }
        }
        
        double _mag; //!< Step magnitude.
        double _scale; //!< Depth scaling factor.
        vector_type _p; //!< Current position.
        vector_type _h; //!< Current heading.
        rotation_matrix_type _Rccw, _Rcw; //! CCW and CW rotation matrices.
    };

    
    //! Tag to select drawing lines.
    struct lineS { };
    
    //! Tag to select drawing points.
    struct pointS { };
    
    /*! 2D turtle for an L-system.
     */
    template
    < typename CoordinateSystem=echo_grid2
    , typename LineSelector=lineS
    , typename LSystem=lsystem<char>
    > class lsystem_turtle2 : public LSystem {
    public:
        typedef LSystem parent;
        typedef CoordinateSystem coor_system_type;
        typedef turtle_context2 context_type;
        typedef std::stack<context_type> context_stack_type;
        typedef std::stack<int> param_stack_type;
        typedef LineSelector line_selector_tag;
        
        //! Constructor.
        lsystem_turtle2() {
            parent::symbol('F')
            .symbol('G')
            .symbol('+')
            .symbol('-')
            .symbol('[')
            .symbol(']')
            .symbol('|');
            
            // push the default context:
            _cstack.push(context_type());
        }
        
        //! Returns the current drawing context.
        context_type& context() { return _cstack.top(); }
        
        //! Draw string s into the given coordinate system.
        void draw(coor_system_type& coor, const typename parent::string_type& s) {
            for(typename parent::string_type::const_iterator i=s.begin(); i!=s.end(); ++i) {
                switch(*i) {
                    case 'F': line(coor); break;
                    case 'G': fwd(coor); break;
                    case '+': ccw(coor); break;
                    case '-': cw(coor); break;
                    case '[': push(coor); break;
                    case ']': pop(coor); break;
                    case '|': scaled_line(coor); break;
                    default: _pstack.push(*i); break;
                }
            }
        }
        
        //! Execute the L-system to a depth of n, and draw its output into coor.
        void draw(coor_system_type& coor, std::size_t n) {
            draw(coor, parent::exec_n(n));
        }
        
    protected:
        //! Returns the top value from the parameter stack, or 1 if none.
        inline int param() {
            int n=1;
            if(!_pstack.empty()) {
                n = _pstack.top();
                _pstack.pop();
            }
            return n;
        }
        
        //! Returns the current context.
        inline context_type& current_context() {
            assert(!_cstack.empty());
            return _cstack.top();
        }
        
        //! Draw a line.
        void draw(coor_system_type& coor, const context_type::point_type& p1, const context_type::point_type& p2, lineS) {
            coor.line(p1, p2);
        }

        //! Draw a point.
        void draw(coor_system_type& coor, const context_type::point_type& p1, const context_type::point_type& p2, pointS) {
            coor.point(p2);
        }

        //! Draw a line from the current position to a single step.
        void line(coor_system_type& coor) {
            context_type& c=current_context();
            context_type::point_type p1=c.point();
            c.step(param());
            context_type::point_type p2=c.point();
            draw(coor, p1, p2, line_selector_tag());
        }
        
        //! Draw a line scaled by the current recursion depth.
        void scaled_line(coor_system_type& coor) {
            context_type& c=current_context();
            context_type::point_type p1=c.point();
            c.step(param(), static_cast<double>(_cstack.size()));
            context_type::point_type p2=c.point();
            draw(coor, p1, p2, line_selector_tag());
        }

        //! Move forward.
        void fwd(coor_system_type& coor) {
            current_context().step(param());
        }
        
        //! Rotate counter-clockwise.
        void ccw(coor_system_type& coor) {
            current_context().rotate_ccw(param());
        }
        
        //! Rotate clockwise.
        void cw(coor_system_type& coor) {
            current_context().rotate_cw(param());
        }
        
        //! Push the current context.
        void push(coor_system_type& coor) {
            _cstack.push(_cstack.top());
        }
        
        //! Pop the current context.
        void pop(coor_system_type& coor) {
            if(_cstack.size() > 1) {
                _cstack.pop();
            }
        }
        
        context_stack_type _cstack; //!< Stack for context.
        param_stack_type _pstack; //!< Stack for parameters.
    };
    
} // ealib

#endif

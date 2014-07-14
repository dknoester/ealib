/* turtle2.h
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
#ifndef _EA_LSYS_TURTLE2_H_
#define _EA_LSYS_TURTLE2_H_

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <deque>

#include <ea/lsys/lsystem.h>

namespace ealib {
    namespace lsys {
        namespace bnu = boost::numeric::ublas;
        
        /*! Simple 2D echoing coordinate system.
         */
        struct echo2 {
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
        
        
        /*! Context for 2D turtles.
         */
        struct turtle_context2 {
            typedef bnu::vector<double> vector_type;
            typedef vector_type point_type;
            typedef bnu::matrix<double> rotation_matrix_type;
            
            //! Default constructor.
            turtle_context2() : _mag(1.0), _scale(1.0) {
                origin(0.0, 0.0);
                heading(1.0, 0.0);
                angle(90.0);
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
            
            //! Sets the angle for both cw (-d) and ccw (+d) rotations.
            turtle_context2& angle(double d) {
                double theta=d*boost::math::constants::pi<double>() / 180.0;
                rotation_matrix(_Rccw, theta);
                rotation_matrix(_Rcw, -theta);
                return *this;
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
        < typename CoordinateSystem=echo2
        , typename LineSelector=lineS
        , typename LSystem=lsystem<char>
        > class lsystem_turtle2 : public LSystem {
        public:
            typedef LSystem parent;
            typedef CoordinateSystem coor_system_type;
            typedef turtle_context2 context_type;
            typedef std::deque<context_type> context_stack_type;
            typedef std::deque<int> param_stack_type;
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
            }
            
            //! Clears the current drawing context.
            void clear() {
                _cstack.clear();
                _pstack.clear();
            }
            
            //! Returns the initial drawing context.
            context_type& context() { return _initial; }
            
            //! Draw string s into the given coordinate system.
            void draw(coor_system_type& coor, const typename parent::string_type& s) {
                clear();
                _cstack.push_back(_initial);
                drawfirst(coor, line_selector_tag());
                
                for(typename parent::string_type::const_iterator i=s.begin(); i!=s.end(); ++i) {
                    switch(*i) {
                        case 'F': line(coor); break;
                        case 'G': fwd(coor); break;
                        case '+': ccw(coor); break;
                        case '-': cw(coor); break;
                        case '[': push(); break;
                        case ']': pop(); break;
                        case '|': scaled_line(coor); break;
                        default: {
                            // if we recognize the symbol, it's a variable that
                            // should be ignored.  otherwise, it's a parameter:
                            if(parent::_V.find(*i) != parent::_V.end()) {
                                _pstack.push_back(*i);
                            }
                            break;
                        }
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
                    n = _pstack.back();
                    _pstack.pop_back();
                }
                return n;
            }
            
            //! Returns the current context.
            inline context_type& current_context() {
                assert(!_cstack.empty());
                return _cstack.back();
            }
            
            //! Initial draw (does nothing for lineS).
            void drawfirst(coor_system_type& coor, lineS) {
            }
            
            //! Initial draw.
            void drawfirst(coor_system_type& coor, pointS) {
                coor.point(current_context().point());
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
            void push() {
                _cstack.push_back(_cstack.back());
            }
            
            //! Pop the current context.
            void pop() {
                if(_cstack.size() > 1) {
                    _cstack.pop_back();
                }
            }
            
            context_type _initial; //!< Initial context.
            context_stack_type _cstack; //!< Stack for context.
            param_stack_type _pstack; //!< Stack for parameters.
        };
        
    } // lsys
} // ealib

#endif

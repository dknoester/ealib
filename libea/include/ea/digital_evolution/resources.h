/* digital_evolution/resources.h
 *
 * This file is part of EALib.
 *
 * Copyright 2012 David B. Knoester, Heather J. Goldsby.
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

#ifndef _EA_DIGITAL_EVOLUTION_RESOURCES_H_
#define _EA_DIGITAL_EVOLUTION_RESOURCES_H_

#include <boost/iterator/iterator_facade.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <utility>
#include <vector>
#include <stdexcept>

#include <ea/algorithm.h>
#include <ea/digital_evolution/position.h>
#include <ea/meta_data.h>


namespace ealib {
    namespace resources {
        
        /*! Abstract resource type.
         */
        struct abstract_resource {
            //! Constructor.
            abstract_resource(const std::string& name) : _name(name) { }
            
            //! Destructor.
            virtual ~abstract_resource() { }
            
            //! Returns the amount of consumed resource.
            virtual double consume(const position_type& pos) = 0;
            
            //! Returns the current resource level.
            virtual double level(const position_type& pos) = 0;

            //! Updates resource levels based on elapsed time since last update (as a fraction of update length).
            virtual void update(double delta_t) { }

            //! Resets resource levels.
            virtual void reset() { }
            
            //! Clears resource levels.
            virtual void clear() { };
            
            //! Returns the name of this resource.
            virtual const std::string& name() { return _name; }
            
            std::string _name; //!< Human-readable name of this resource.
        };
        
        /*! Unlimited resource type.
         
         This resource type is neither limited nor spatial; individuals are always
         able to consume this resource, regardless of their position.
         */
        struct unlimited : abstract_resource {
            //! Constructor.
            unlimited(const std::string& name) : abstract_resource(name) { }

            //! Destructor.
            virtual ~unlimited() { }

            //! Returns the amount of consumed resource.
            virtual double consume(const position_type& pos) { return 1.0; }

            //! Returns the current resource level.
            virtual double level(const position_type& pos) { return 1.0; }
        };
        
        /*! Limited resource type.
         
         This resource is limited, but not spatial.  Individuals are able to consume
         this resource based on its current level, regardless of their position.
         Additional resources flow in over time (not all at once).
         
         This resource type is roughly akin to a chemostat.
         */
        template <typename EA>
        struct limited : abstract_resource {
            //! Constructor.
            limited(const std::string& name, double initial, double inflow, double outflow, double consume)
            : abstract_resource(name), _initial(initial), _level(initial), _inflow(inflow), _outflow(outflow), _consume(consume) {
            }
            
            //! Destructor.
            virtual ~limited() { }
            
            //! Returns the amount of consumed resource.
            virtual double consume(const position_type& pos) {
                double r = std::max(0.0, _level*_consume);
                _level = std::max(0.0, _level-r);
                return r;
            }
            
            //! Returns the current resource level.
            virtual double level(const position_type& pos) { return _level; }
            
            //! Updates resource levels based on elapsed time since last update (as a fraction of update length).
            virtual void update(double delta_t) {
                _level += delta_t * (_inflow - (_outflow * _level));
                _level = std::max(0.0, _level);
            }
            
            //! Resets resource levels.
            virtual void reset() { _level = _initial; }
            
            //! Clears resource levels.
            virtual void clear() { _level = 0.0; }
            
            double _initial; //!< Initial resource level
            double _level; //!< Current resource level.
            double _inflow; //!< Amount of resource flowing in per update.
            double _outflow; //!< Rate at which resource flows out per update.
            double _consume; //!< Fraction of resource consumed.
        };
        
        /*! Spatial resource type.
         
         This resource is both limited and spatial.  Individuals consume this resource
         based on their location and current resource levels.  Additional resources
         flow in over time (not all at once), and they diffuse throughout the environment.
         
         \note We assume a 2D discrete environment.
         */
        struct spatial : abstract_resource {
            typedef boost::numeric::ublas::matrix<double> matrix_type; //!< Type for matrix that will store resource levels.
            
            //! Constructor.
            spatial(const std::string& name, double diffuse, double initial,
                    double inflow, double outflow, double consume,
                    std::size_t x, std::size_t y)
                : abstract_resource(name)
                , _diffuse(diffuse), _initial(initial), _level(initial)
                , _inflow(inflow), _outflow(outflow), _consume(consume) {
                _R.resize(y,x);
                _T.resize(y,x);
                reset();
            }
            
            //! Destructor.
            virtual ~spatial() { }

            //! Returns the amount of consumed resource.
            virtual double consume(const position_type& pos) {
                double level = _R(pos[0], pos[1]);
                double r = std::max(0.0, level*_consume);
                _R(pos[0], pos[1]) = std::max(0.0, level-r);
                return r;
            }

            //! Returns the current resource level.
            virtual double level(const position_type& pos) {
                return _R(pos[0], pos[1]);
            }

            //! Updates resource levels based on elapsed time since last update (as a fraction of update length).
            void update(double delta_t) {
                // for stability...
                assert(delta_t < (1.0/(2.0*_diffuse)));
                
                // last row and column indices...
                std::size_t ni=_R.size1()-1;
                std::size_t nj=_R.size2()-1;
                
                // inflow to the top row...
                for(std::size_t j=0; j<nj; ++j) {
                    _R(0,j) += _inflow;
                }
                
                // outflow from the bottom row...
                for(std::size_t j=0; j<nj; ++j) {
                    _R(ni,j) = std::max(0.0, _R(ni,j) - _outflow);
                }
                
                // diffuse
                // based on: http://www.timteatro.net/2010/10/29/performance-python-solving-the-2d-diffusion-equation-with-numpy/
                for(std::size_t i=1; i<ni; ++i) {
                    for(std::size_t j=1; j<nj; ++j) {
                        double uxx = _R(i+1,j) - 2*_R(i,j) + _R(i-1,j);
                        double uyy = _R(i,j+1) - 2*_R(i,j) + _R(i,j-1);
                        _T(i,j) = _R(i,j) + delta_t * _diffuse * (uxx+uyy);
                    }
                }
                _R.swap(_T);
            }
            
            //! Resets resource levels.
            void reset() {
                _R = boost::numeric::ublas::scalar_matrix<double>(_R.size1(), _R.size2(), _initial);
                _T = boost::numeric::ublas::scalar_matrix<double>(_R.size1(), _R.size2(), _initial);
            }
            
            //! Clears resource levels.
            void clear() {
                _R = boost::numeric::ublas::scalar_matrix<double>(_R.size1(), _R.size2(), 0.0);
                _T = boost::numeric::ublas::scalar_matrix<double>(_R.size1(), _R.size2(), 0.0);
            }
            
            
            matrix_type _R; //!< Matrix for current resource levels at each cell.
            matrix_type _T; //!< Matrix for updating resource levels at each cell.
            double _diffuse; //!< Diffusion constant for this resource.
            double _initial; //!< Initial resource level
            double _level; //!< Current resource level.
            double _inflow; //!< Amount of resource flowing in per update.
            double _outflow; //!< Rate at which resource flows out per update.
            double _consume; //!< Fraction of resource consumed.
        };
        
    } // resources
    
    //! Helper method that builds an unlimited resource and adds it to the environment.
    template <typename EA>
    typename EA::environment_type::resource_ptr_type make_resource(const std::string& name, EA& ea) {
        typedef typename EA::environment_type::resource_ptr_type resource_ptr_type;
        resource_ptr_type p(new resources::unlimited(name));
        ea.env().add_resource(p);
        return p;
    }
    
    //! Helper method that builds a limited resource and adds it to the environment.
    template <typename EA>
    typename EA::environment_type::resource_ptr_type make_resource(const std::string& name, double initial, double inflow, double outflow, double consume, EA& ea) {
        typedef typename EA::environment_type::resource_ptr_type resource_ptr_type;
        resource_ptr_type p(new resources::limited<EA>(name, initial, inflow, outflow, consume));
        ea.env().add_resource(p);
        return p;
    }
    
//    //! Helper method that builds a spatial resource and adds it to the environment.
//    template <typename EA>
//    typename EA::environment_type::resource_ptr_type make_resource(const std::string& name,
//                                                                   double diffuse,
//                                                                   double initial, double inflow, double outflow, double consume, EA& ea) {
//        typedef typename EA::environment_type::resource_ptr_type resource_ptr_type;
//        resource_ptr_type p(new resources::spatial(name, diffuse, initial, inflow, outflow, consume, get<SPATIAL_X>(ea), get<SPATIAL_Y>(ea)));
//        ea.env().add_resource(p);
//        return p;
//    }

} // ealib

#endif

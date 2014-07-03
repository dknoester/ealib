/* filter.h
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
#ifndef _EA_FILTER_H_
#define _EA_FILTER_H_

#include <cmath>
#include <functional>

namespace ann {
    
    /*! These unary function objects are for applying "filters" to values, e.g.,
     for scaling a variable to be within a given range.
     */
    
    template <typename T>
    struct step : std::unary_function<T,T> {
        typedef std::unary_function<T,T> base_type;
        typedef typename base_type::argument_type argument_type;
        typedef typename base_type::result_type result_type;
        
        step(T lv, T ip, T uv) : _lv(lv), _ip(ip), _uv(uv) { }
        
        result_type operator()(argument_type x) {
            if(x > _ip) {
                return _uv;
            } else {
                return _lv;
            }
        }
        
        T _lv, _ip, _uv; //!< lower value, inflection point, upper value
    };
    
    template <typename T>
    struct clip : std::unary_function<T,T> {
        typedef std::unary_function<T,T> base_type;
        typedef typename base_type::argument_type argument_type;
        typedef typename base_type::result_type result_type;
        
        clip(T lt, T lv, T ut, T uv) : _lt(lt), _lv(lv), _ut(ut), _uv(uv) {
        }
        
        result_type operator()(argument_type x) {
            if(x >= _ut) {
                return _uv;
            } else if(x <= _lt) {
                return _lv;
            } else {
                return x;
            }
        }
        
        T _lt, _lv, _ut, _uv; //!< lower and upper thresholds and values.
    };
    
    template <typename T>
    struct identity : std::unary_function<T,T> {
        typedef std::unary_function<T,T> base_type;
        typedef typename base_type::argument_type argument_type;
        typedef typename base_type::result_type result_type;
        
        result_type operator()(argument_type x) {
            return x;
        }
    };
	
} // ann

#endif

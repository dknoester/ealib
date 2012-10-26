/* svm.h
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
#ifndef _MKV_STATE_VECTOR_MACHINE_H_
#define _MKV_STATE_VECTOR_MACHINE_H_

namespace mkv {
    
    /*! State Vector Machine.
     
     Encapsulates much of the common functionality needed for state vector-based
     machines.
     */
    template <typename StateType>
    class state_vector_machine {
    public:
        typedef StateType state_type;
        typedef std::vector<state_type> state_vector_type;

        //! Constructor.
        state_vector_machine(std::size_t n) : _n(n), _sv0(n), _sv1(n) {
            _t = &_sv0;
            _tminus1 = &_sv1;
        }        
         
        //! Retrieves the size of this SVM, in number of states.
        std::size_t size() const {
            return _n;
        }
        
        //! Resize the SVM by adding or removing states at the end as needed.
        void resize(std::size_t n) {
            _n = n;
            _sv0.resize(n);
            _sv1.resize(n);
        }
        
        //! Rotate t to t-1, and reset states in t to their default.
        void rotate() {
            std::swap(_tminus1, _t);
            std::fill(_t->begin(), _t->end(), state_type());
        }
        
        //! Clear the state vectors.
        void clear() { 
            std::fill(_tminus1->begin(), _tminus1->end(), state_type());
            std::fill(_t->begin(), _t->end(), state_type());
        }
        
        //! Retrieve the state vector at time t.
        state_vector_type& t() { return *_t; }
        
        //! Retrieve the state vector at time t-1.
        state_vector_type& tminus1() { return *_tminus1; }
        
        //! Retrieve state n at time t.
        state_type& state_t(std::size_t n) { return (*_t)[n]; }

        //! Retrieve state n at time t-1.
        state_type& state_tminus1(std::size_t n) { return (*_tminus1)[n]; }
        
        //! Retrieve state n at time t (const-qualified).
        const state_type& state_t(std::size_t n) const { return (*_t)[n]; }
        
        //! Retrieve state n at time t-1 (const-qualified).
        const state_type& state_tminus1(std::size_t n) const { return (*_tminus1)[n]; }

    protected:
        std::size_t _n; //!< Number of states.
        state_vector_type _sv0; //!< A state vector.
        state_vector_type _sv1; //!< A state vector.
        state_vector_type* _t; //!< Pointer to state vector at time t.
        state_vector_type* _tminus1; //!< Pointer to state vector at time t-1.
    };

} // mkv

#endif

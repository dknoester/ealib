/* torus2.h
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
#ifndef _EA_TORUS2_H_
#define _EA_TORUS2_H_

#include <boost/numeric/ublas/matrix.hpp>

namespace ealib {
    
    /*! 2-dimensional toroidal container.
     */
    template <typename T>
    class torus2 : public boost::numeric::ublas::matrix<T> {
    public:
        typedef boost::numeric::ublas::matrix<T> parent;
        typedef typename parent::value_type value_type;
        typedef typename parent::reference reference;
        typedef typename parent::const_reference const_reference;
        
        //! Default constructor.
        torus2() {
        }
        
        //! Constructor.
        torus2(std::size_t m, std::size_t n, const T& t=T()) : parent(m,n,t) {
        }
        
        //! Convenience method to fill this torus with a range of values.
        template <typename ForwardIterator>
        void fill(ForwardIterator f, ForwardIterator l) {
            for(std::size_t i=0; i<parent::size1(); ++i) {
                for(std::size_t j=0; j<parent::size2(); ++j) {
                    if(f == l) {
                        return;
                    }
                    parent::operator()(i,j) = *f++;
                }
            }
        }
        
        //! Returns a reference to element (i,j).
        reference operator()(int i, int j) {
            return parent::operator()(rebase(i,parent::size1()), rebase(j,parent::size2()));
        }

        //! Returns a reference to element (i,j) (const-qualified).
        const_reference operator()(int i, int j) const {
            return parent::operator()(rebase(i,parent::size1()), rebase(j,parent::size2()));
        }
        
    protected:
        //! Rebase index x to size y.
        inline std::size_t rebase(int x, std::size_t y) const {
            if(x >= 0) {
                return x % y;
            } else {
                x = (-1*x) % y;
                return y - x;
            }
        }
    };

} // ea

#endif

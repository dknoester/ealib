/* matrix.h
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
#ifndef _MKV_MATRIX_H_
#define _MKV_MATRIX_H_

namespace mkv {
    
    /*! Adaptor that converts a Sequence into a boost::Matrix
     */
    template <typename Sequence>
    class sequence_matrix {
    public:
        typedef typename Sequence::value_type value_type;
        
        //! Constructor.
        sequence_matrix(Sequence& s, std::size_t s1, std::size_t s2) : _s(s), _size1(s1), _size2(s2) {
        }
        
        //! Retrieve element (i,j) from the matrix.
        value_type& operator()(std::size_t i, std::size_t j) {
            return _s[index(i,j)];
        }
        
        //! Calculate the index of element (i,j).
        inline std::size_t index(std::size_t i, std::size_t j) {
            assert(i<_size1);
            assert(j<_size2);
            std::size_t x = i*_size2 + j;
            assert(x < _s.size());
            return x;
        }
        
        //! Size of dimension 1 (# of rows).
        std::size_t size1() { return _size1; }
        
        //! Size of dimension 2 (# of columns).
        std::size_t size2() { return _size2; }
        
    protected:
        Sequence& _s; //!< Sequence being adapted.
        std::size_t _size1, _size2; //!< Sizes of the matrix (m,n)
    };
    
    //! Output the camera's retina.
    template <typename Sequence>
    std::ostream& operator<<(std::ostream& out, sequence_matrix<Sequence>& M) {
        for(std::size_t i=0; i<M.size1(); ++i) {
            for(std::size_t j=0; j<M.size2(); ++j) {
                out << M(i,j);
            }
            out << std::endl;
        }
        return out;
    }

    
    
    /*! Random access Matrix iterator, used to bridge Matrix input to a random
     access iterator suitable for Markov networks.
     
     Specifically, given a random_access_matrix_iterator ra over m x n matrix M, 
     ra[i] returns the value at M_{i/n, i%n}.
     */
    template <typename Matrix>
    struct random_access_matrix_iterator {
        //! Constructor.
        random_access_matrix_iterator(Matrix& M) : _M(M) {
        }
        
        //! Returns M_{i/n,i%n}.
        typename Matrix::value_type operator[](std::size_t i) {
            return _M(i/_M.size2(), i % _M.size2());
        }
        
        Matrix& _M; //!< Matrix being iterated over.
    };
        

    /*! Unary matrix function that calls a Markov network on a matrix, and returns
     the value output by the Markov network.
     */
    template <typename Network>
    struct matrix_call {
        //! Constructor.
        matrix_call(Network& net, int n) : _net(net), _n(n) {
        }
        
        /*! Call the embedded Markov network on matrix M, returning the Markov network's
         output.
         */
        template <typename Matrix>
        typename Matrix::value_type operator()(Matrix& M) {
            mkv::update(_net, _n, random_access_matrix_iterator<Matrix>(M));
            return ealib::algorithm::range_pair2int(_net.begin_output(), _net.end_output());
        }
        
        Network& _net; //!< Network to be called on a matrix.
        int _n; //!< Number of times to update the network.
    };

} // mkv

#endif

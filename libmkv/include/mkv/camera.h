/* convolve_data.h
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
#ifndef _MKV_CAMERA_H_
#define _MKV_CAMERA_H_

#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/storage.hpp>
#include <algorithm>
#include <iostream>
#include <ea/algorithm.h>

namespace mkv {
    
    //! Used to select the axis being moved.
    enum axis_type { X_AXIS, Y_AXIS, Z_AXIS };
 
    /*! 2D camera-based iterator over a Matrix.

     Consider a 2D Cartesian coordinate plane:
     
     Y+
     ^
     | UR     LR
     |   image
     | UL     LL
     + ----->
        X+
     
     We place an image in this plane such that the upper-left corner of the image
     is at (0,0) in the plane.  (We do this to avoid potential hard-to-find bugs
     related to image transformations between matrix and Cartesian coordinates.)
     
     We then define a camera that is simply a restricted-size region of this image,
     a.k.a., a matrix range, and then allow random access iteration over this range.
     */
    template <typename Matrix>
    struct camera2_iterator {
        //! Constructor.
        camera2_iterator(Matrix& M, std::size_t s1, std::size_t s2, std::size_t i=0, std::size_t j=0) : _M(M), _size1(s1), _size2(s2), _i(i), _j(j) {
        }
        
        //! Retrieve the value of the n'th entry in the camera's retina.
        typename Matrix::value_type operator[](std::size_t n) {
            assert(n < (_size1*_size2));
            std::size_t i=_i + n/_size2;
            std::size_t j=_j + n%_size2;
            
            assert(i < _M.size1());
            assert(j < _M.size2());
            return _M(i,j);
        }
        
        //! Place the camera at absolute position (i,j).
        void position(std::size_t i, std::size_t j) {
            _i = i;
            _j = j;
            clip();
        }
        
        //! Move the camera by (i,j), relative to its current position.
        void move(int i, int j) {
            _i += i;
            _j += j;
            clip();
        }
        
        //! Clip the camera's position to the bounds of the image.
        void clip() {
            _i = ealib::algorithm::clip(_i, 0ul, _M.size1()-_size1);
            _j = ealib::algorithm::clip(_j, 0ul, _M.size2()-_size2);
        }
        
        std::size_t size() { return _size1 * _size2; }
        
        Matrix& _M; //!< Image that we're iterating over.
        std::size_t _size1, _size2; //!< Sizes of the camera's retina (m,n)
        std::size_t _i; //!< Row position in _M of the camera.
        std::size_t _j; //!< Column position in _M of the camera.
    };

    //! Output the camera's retina.
    template <typename Matrix>
    std::ostream& operator<<(std::ostream& out, camera2_iterator<Matrix>& ci) {
        for(std::size_t i=0; i<(ci._size1*ci._size2); ++i) {
            out << ci[i] << " ";
        }
        return out;
    }    

    
    /*! 3D camera-based iterator over a Matrix.
     
     Consider a 3D Cartesian coordinate system, following the right-hand-rule:
     
     ....^
     ....|
     ....| Z+
     ....|
     ....|    Y+
     ....x---------->
     .../
     ../ X+    (UL        UR)
     ./          image
     v       (LL        LR)
     
     Let us then place an image in the X-Y plane, with the origin (0,0,0) being
     the upper left corner of the image.  A camera is then simply a point in
     this space that projects a retina down from its position to the X-Y
     plane.  Conceptually, a camera is simply a multi-axis device that can be
     moved around in this space.
     
     The camera itself comprises a retina of pixels with the same orientation as
     the image:
     
     Y
     ^
     | UR               LR
     |   camera & image
     | UL               LL
     + -----> X
     
     Moving about the image in the X & Y directions is straightforward.  We assume
     that all "motors" for X & Y operate in X-Y plane; that is, a move of (1x,1y)
     means, move the camera 1 pixel right and 1 pixel up in the X-Y plane.
     
     --------
     Height (Z-axis)
     
     In the Z direction, things get a little tricky.  Here we change the
     semantics of Z from a pixel position to a sort of "height" value.  Specifically,
     the camera's Z position refers to the width of the (square) region in the image
     that is used to calculate a pixel value (via max-pooling).
     
     In other words, if the camera is at Z=2, a 2x2 square in the X-Y plane is used
     to calculate the value of each sensor in the camera's retina. If Z=3, a 3x3
     square is used, and so on.
     
     --------
     Limits
     
         min   max
     X  -inf  +inf
     Y  -inf  +inf
     Z   1    +inf

     --------
     Random access
     
     This iterator supports random-access, where indices refer to the sensors in
     the camera's retina.  In the case of Z values > 1, the sensor value is 
     calculated via max-pooling.  Note that the camera's retina can be "off" the
     image -- that is, the camera is not bound by the size of the image.  In this
     case, sensors that are off the image are always 0.
     
     \todo Alter this to do raycasting for each retina sensor.

     \todo Camera's are multi-axis devices, and it is clear that at least 6 axes
     make sense (pitch, yaw, and roll).  We do not deal with these yet.
     */
    template <typename Matrix>
    struct camera3_iterator {

        //! Constructor.
        camera3_iterator(Matrix& M, std::size_t s1, std::size_t s2) : _M(M), _size1(s1), _size2(s2), _x(0), _y(0), _z(1) {
        }
        
        //! Retrieve the maximum value in the n'th (_z x _z) matrix in the camera's retina.
        typename Matrix::value_type operator[](std::size_t n) {
            // sensor i and j:
            int ri=n/_size2;
            int rj=n%_size2;
            
            // anchor point in image coordinates:
            int ai=_y + ri*_z;
            int aj=_x + rj*_z;
            
            // now, iterate over the intersection between (ai,ai+k,aj,aj+k) and
            // the image:
            typename Matrix::value_type t=static_cast<typename Matrix::value_type>(0);
            
            for(int a=std::max(0,ai); (a<(ai+_z)) && (a<static_cast<int>(_M.size1())); ++a) {
                for(int b=std::max(0,aj); (b<(aj+_z)) && (b<static_cast<int>(_M.size2())); ++b) {
                    t = std::max(t, _M(a,b));
                }
            }
            
            return t;
        }

        //! Move the camera d units along axis a.
        void move(axis_type a, int d) {
            switch(a) {
                case X_AXIS: {
                    _x += d;
                    break;
                }
                case Y_AXIS: {
                    _y += d;
                    break;
                }
                case Z_AXIS: {
                    _z = std::max(_z+d, 1);
                    break;
                }
            }
        }

        //! Move the camera d*z units along axis a.
        void scaled_move(axis_type a, int d) {
            switch(a) {
                case X_AXIS: {
                    _x += d*_z;
                    break;
                }
                case Y_AXIS: {
                    _y += d*_z;
                    break;
                }
                case Z_AXIS: {
                    _z = std::max(_z+d*_z, 1);
                    break;
                }
            }
        }

        //! Move the camera (i,j) units along axes (y,x).
        void move_ij(int i, int j) {
            move(X_AXIS,j);
            move(Y_AXIS,i);
        }

        //! Move the camera (x,y) units.
        void move_xy(int x, int y) {
            move(X_AXIS,x);
            move(Y_AXIS,y);
        }

        //! Home the given axis.
        void home(axis_type a) {
            switch(a) {
                case X_AXIS: {
                    _x = 0;
                    break;
                }
                case Y_AXIS: {
                    _y = 0;
                    break;
                }
                case Z_AXIS: {
                    _z = 1;
                    break;
                }
            }
        }

        //! Home all axes.
        void home() {
            home(X_AXIS);
            home(Y_AXIS);
            home(Z_AXIS);
        }
        
        Matrix& _M; //!< Matrix being "viewed."
        std::size_t _size1, _size2; //!< Sizes of the camera's retina (m,n)
        int _x; //!< Column position in _M of the camera.
        int _y; //!< Row position in _M of the camera.
        int _z; //!< Height of the camera above _M.
    };
    
    //! Output the camera's retina.
    template <typename Matrix>
    std::ostream& operator<<(std::ostream& out, camera3_iterator<Matrix>& ci) {
        for(std::size_t i=0; i<(ci._size1*ci._size2); ++i) {
            out << ci[i] << " ";
        }
        return out;
    }

} // mkv

#endif

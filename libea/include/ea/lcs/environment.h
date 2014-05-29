/* environment.h
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
#ifndef _EA_LEARNING_CLASSIFIER_ENVIRONMENT_H_
#define _EA_LEARNING_CLASSIFIER_ENVIRONMENT_H_

namespace ealib {

    /*! The environment in an LCS is technically an interface between the classifiers
     in the LCS' population and the extenal world.  It communicates with the LCS via
     messages that are passed via detectors and effectors.
     */
    struct abstract_environment {
        template <typename EA>
        void detectors(typename EA::message_board_type& mb, EA& ea) {
        }
        
        template <typename EA>
        void effectors(typename EA::message_board_type& mb, EA& ea) {
        }
        
        //! Serialize this fitness function.
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version) {
        }
    };
    
} // ea

#endif

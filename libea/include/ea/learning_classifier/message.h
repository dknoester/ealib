/* action.h
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
#ifndef _EA_LEARNING_CLASSIFIER_MESSAGE_H_
#define _EA_LEARNING_CLASSIFIER_MESSAGE_H_

#include <ea/representations/bitstring.h>
namespace ealib {
    
    //! Message class; transmitted between environment, internal message board, and rules.
    struct message : public bitstring {
        typedef bitstring base_type;

        message() : base_type() {
        }

        message(const bitstring& that) : base_type(that) {
        }
        
        template <typename Individual>
        void consumed_by(Individual& ind) {
            
        }
    };
    
    /*! Message board class.
     */
    class message_board : public std::vector<message> {
    public:
        typedef message message_type;
        
    protected:
        
    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
        }
    };
    
} // ea

#endif

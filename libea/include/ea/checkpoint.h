/* checkpoint.h
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
#ifndef _EA_CHECKPOINT_H_
#define _EA_CHECKPOINT_H_

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ea/meta_data.h>


namespace ealib {
//
//	template <typename EA>
//	void checkpoint(EA& ea) {
//		std::ostringstream filename;
//		filename << get<CHECKPOINT_PREFIX>(ea) << "-" << ea.current_update() << ".xml";
//		std::ofstream out(filename.str().c_str());
//		checkpoint_save(ea, out);
//	}
//	
//	template <typename EA>
//	void checkpoint_save(EA& ea, std::ostream& out) {
//		boost::archive::xml_oarchive oa(out);
//		oa << BOOST_SERIALIZATION_NVP(ea);		
//	}
//	
//	template <typename EA>
//	void checkpoint_load(EA& ea, std::istream& in) {
//		boost::archive::xml_iarchive ia(in);
//		ia >> BOOST_SERIALIZATION_NVP(ea);
//	}

} // ea

#endif

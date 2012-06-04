#ifndef _EA_CHECKPOINT_H_
#define _EA_CHECKPOINT_H_

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ea/meta_data.h>


namespace ea {

	template <typename EA>
	void checkpoint(EA& ea) {
		std::ostringstream filename;
		filename << get<CHECKPOINT_PREFIX>(ea) << "-" << ea.current_update() << ".xml";
		std::ofstream out(filename.str().c_str());
		checkpoint_save(ea, out);
	}
	
	template <typename EA>
	void checkpoint_save(EA& ea, std::ostream& out) {
		boost::archive::xml_oarchive oa(out);
		oa << BOOST_SERIALIZATION_NVP(ea);		
	}
	
	template <typename EA>
	void checkpoint_load(EA& ea, std::istream& in) {
		boost::archive::xml_iarchive ia(in);
		ia >> BOOST_SERIALIZATION_NVP(ea);
	}

} // ea

#endif

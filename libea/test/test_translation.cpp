/* test_circular.cpp
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
#ifndef BOOST_TEST_DYN_LINK
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include <ea/translation.h>

template <typename Genome, typename Phenotype>
struct symbol_gene {
	virtual void operator()(typename Genome::iterator f, Phenotype& P) {
		P.symbol(*f);
	}
};

template <typename Genome, typename Phenotype>
struct axiom_gene {
	virtual void operator()(typename Genome::iterator f, Phenotype& P) {
		P.symbol(*f);
	}
};

template <typename Genome, typename Phenotype>
struct rule_gene {
	virtual void operator()(typename Genome::iterator f, Phenotype& P) {
		typename Phenotype::string_type s;
		P.rule(*f, s);
	}
};

template <typename Genome, typename Phenotype>
class lsys_translator : public ealib::translator<Genome, Phenotype> {
public:
	typedef ealib::translator<Genome, Phenotype> parent;
	
	template <typename EA>
	lsys_translator(EA& ea) {
		parent::template add_gene<symbol_gene>(ea);
		parent::template add_gene<axiom_gene>(ea);
		parent::template add_gene<rule_gene>(ea);

	}
};

BOOST_AUTO_TEST_CASE(test_translation) {
	using namespace ealib;
	


}

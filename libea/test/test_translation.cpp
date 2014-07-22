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
#include <boost/test/unit_test.hpp>
#include <ea/translation.h>
#include <ea/lsys/lsystem.h>
#include <ea/genome_types/circular_genome.h>
#include <ea/evolutionary_algorithm.h>
#include <ea/genome_types/bitstring.h>
#include <ea/fitness_functions/all_ones.h>
#include <ea/generational_models/steady_state.h>
#include <ea/line_of_descent.h>
using namespace ealib;
using namespace ealib::lsys;

typedef evolutionary_algorithm
< direct<bitstring>
, all_ones
, mutation::operators::per_site<mutation::site::bitflip>
, recombination::two_point_crossover
, generational_models::steady_state< >
, ancestors::random_bitstring
> all_ones_ea;

//SYMBOL: start_codon | symbol
//AXIOM: start_codon | symbol
//RULE: start_codon | symbol | size | symbol*

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
        std::size_t n=*(f+1);
		typename Phenotype::string_type s(f+1, f+1+n);
		P.rule(*f, s);
	}
};

template <typename Genome, typename Phenotype>
class lsys_translator : public ealib::translator<Genome, Phenotype> {
public:
	typedef ealib::translator<Genome, Phenotype> parent;
	
	template <typename EA>
	lsys_translator(EA& ea) : parent(ea) {
		parent::template add_gene<symbol_gene>(ea);
		parent::template add_gene<axiom_gene>(ea);
		parent::template add_gene<rule_gene>(ea);
	}
};

BOOST_AUTO_TEST_CASE(test_translation) {
    all_ones_ea ea;
    typedef lsystem<char> lsys_type;
    lsys_type L;
    
    //    L.symbol('A')
    //    .symbol('B')
    //    .axiom(L.string('A'))
    //    .rule('A', L.splitc("AB"))
    //    .rule('B', L.string('A'));
    
    circular_genome<char> G(100, 127);
    int i=10;
    G[++i] = 0;
    G[++i] = 255;
    G[++i] = 'A'; // symbol
    G[++i] = 0;
    G[++i] = 255;
    G[++i] = 'B'; // symbol
    G[++i] = 1;
    G[++i] = 254;
    G[++i] = 'A'; // axiom
    G[++i] = 2;
    G[++i] = 253;
    G[++i] = 'A';
    G[++i] = 2;
    G[++i] = 'A';
    G[++i] = 'B'; // rule
    G[++i] = 2;
    G[++i] = 253;
    G[++i] = 'B';
    G[++i] = 1;
    G[++i] = 'A'; // rule
    
    translator<circular_genome<char>, lsys_type> T(ea);
    T(G, L);
    
    lsys_type::string_type s = L.exec_n(7);
    const std::string t("ABAABABAABAABABAABABAABAABABAABAAB");
    BOOST_CHECK(std::equal(s.begin(), s.end(), t.begin()));
}

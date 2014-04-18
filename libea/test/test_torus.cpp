/* test_circular.cpp
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
#ifndef BOOST_TEST_DYN_LINK
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include <ea/algorithm.h>
#include <ea/cvector.h>
#include <ea/torus.h>

BOOST_AUTO_TEST_CASE(test_torus1) {
	using namespace ealib;
    typedef std::vector<int> ivector;
    typedef torus1_proxy<ivector> itorus;
    ivector v(10);
    BOOST_CHECK_EQUAL(v.size(), 10);
    algorithm::iota(v.begin(), v.end());

    itorus t(v);
    BOOST_CHECK_EQUAL(t.size(), v.size());
    
    
    itorus::iterator i=t.begin();
    for(int j=0; j<(t.size()*2); ++i,++j) {
        BOOST_CHECK_EQUAL(v[j%v.size()], t[j]);
        BOOST_CHECK_EQUAL(t[j], *i);
    }
    
    i = t.begin();
    i += 5;
    ivector w(i,i+10);
    for(int j=0; j<w.size(); ++j) {
        BOOST_CHECK_EQUAL(w[j], (j+5)%w.size());
    }
    
    t.erase(t.begin()+8, t.begin()+13);
    BOOST_CHECK_EQUAL(t.size(), 5);
    
    t.erase(t.begin(), t.begin()+2);
    BOOST_CHECK_EQUAL(t.size(), 3);
    
//    
//    // insertion:
//    if(((repr.size()+get<MUTATION_INDEL_MIN_SIZE>(ea)) < static_cast<std::size_t>(get<REPRESENTATION_MAX_SIZE>(ea)))
//       && ea.rng().p(get<MUTATION_INSERTION_P>(ea))) {
//        torus1_proxy<typename EA::representation_type> t(repr);
//        torus1_proxy<typename EA::representation_type>::iterator src=ea.rng().choice(t.begin(), t.end());
//        
//        std::size_t chunk_size = std::min(get<REPRESENTATION_MAX_SIZE>(ea) - repr.size(),
//                                          ea.rng()(get<MUTATION_INDEL_MIN_SIZE>(ea), get<MUTATION_INDEL_MAX_SIZE>(ea)));
//        typename EA::representation_type chunk(src, src+chunk_size);
//        repr.insert(ea.rng().choice(repr.begin(),repr.end()), chunk.begin(), chunk.end());
//    }
//    
//    // deletion:
//    if(((repr.size()-get<MUTATION_INDEL_MIN_SIZE>(ea)) > static_cast<std::size_t>(get<REPRESENTATION_MIN_SIZE>(ea)))
//       && ea.rng().p(get<MUTATION_DELETION_P>(ea))) {
//        torus1_proxy<typename EA::representation_type> t(repr);
//        torus1_proxy<typename EA::representation_type>::iterator src=ea.rng().choice(t.begin(), t.end());
//        
//        std::size_t chunk_size = std::min(repr.size() - get<REPRESENTATION_MIN_SIZE>(ea),
//                                          ea.rng()(get<MUTATION_INDEL_MIN_SIZE>(ea), get<MUTATION_INDEL_MAX_SIZE>(ea)));
//        t.erase(src, src+chunk_size);
//    }

}


BOOST_AUTO_TEST_CASE(test_torus3) {
	using namespace ealib;
    torus3<int> T(3,3,3);
    algorithm::iota(T.begin(), T.end());
    
    // page 0:
    // 0 1 2
    // 3 4 5
    // 6 7 8
    //
    // page 1:
    // 9 10 11
    // 12 13 14
    // 15 16 17
    //
    // page 2:
    // 18 19 20
    // 21 22 23
    // 24 25 26
    
    BOOST_CHECK(T.size()==27);
    BOOST_CHECK(T(0,0,0)==0);
    BOOST_CHECK(T(2,2,2)==26);
    BOOST_CHECK(T(-1,0,0)==6);
    BOOST_CHECK(T(0,-1,0)==2);
    BOOST_CHECK(T(0,0,-1)==18);
    
    offset_torus3<torus3<int> > O(&T,-1,-1,-1);
    BOOST_CHECK(O(0,0,0)==26);
    BOOST_CHECK(O(1,1,1)==0);
    
    adaptor_torus3<offset_torus3<torus3<int> > > A(O, 2, 2, 2);
    BOOST_CHECK(A[0]==26);
    BOOST_CHECK(A[1]==24);
    BOOST_CHECK(A[2]==20);
    BOOST_CHECK(A[3]==18);
    BOOST_CHECK(A[4]==8);
    BOOST_CHECK(A[5]==6);
    BOOST_CHECK(A[6]==2);
    BOOST_CHECK(A[7]==0);
    
    adaptor_torus3<offset_torus3<torus3<int> > > B(O, 3, 3, 3);
    BOOST_CHECK(B[0]==26);
    BOOST_CHECK(B[1]==24);
    BOOST_CHECK(B[2]==25);
    BOOST_CHECK(B[3]==20);
    BOOST_CHECK(B[4]==18);
    BOOST_CHECK(B[5]==19);
    BOOST_CHECK(B[6]==23);
    BOOST_CHECK(B[7]==21);
    BOOST_CHECK(B[8]==22);

    
}


BOOST_AUTO_TEST_CASE(test_torus2) {
	using namespace ealib;
    torus2<int> T(3,3);
    algorithm::iota(T.begin(), T.end());
    // 0 1 2
    // 3 4 5
    // 6 7 8
    
    BOOST_CHECK(T.size()==9);
    BOOST_CHECK(T(0,0)==0);
    BOOST_CHECK(T(2,2)==8);
    BOOST_CHECK(T(-1,0)==6);
    BOOST_CHECK(T(4,4)==4);
    
    offset_torus2<torus2<int> > O(&T,-1,-1);
    BOOST_CHECK(O(0,0)==8);
    BOOST_CHECK(O(0,-1)==7);
    
    adaptor_torus2<offset_torus2<torus2<int> > > A(O, 2, 2);
    BOOST_CHECK(A[0]==8);
    BOOST_CHECK(A[1]==6);
    BOOST_CHECK(A[2]==2);
    BOOST_CHECK(A[3]==0);
}

BOOST_AUTO_TEST_CASE(test_circular_vector) {
	using namespace ealib;

    int data[33]={
		44, 255-44, // start
		1, 8, // 2in, 2out
        0, // history size (1)
        3, 3, // posf, negf
		0, 1, // inputs
		2, 3, // outputs
        32767, // poswv
        0, // negwv
		10, 0, 0, 0, // P table
		0, 10, 0, 0,
		0, 0, 10, 0,
		0, 0, 0, 10
	};

    typedef cvector<int> cv_type;
    cv_type cv(data, data+12);
    BOOST_CHECK(cv.size()==12);
    BOOST_CHECK(*cv.rbegin()==32767);
    
    cv_type::iterator i=cv.begin();
    BOOST_CHECK(*i==44);
    
    std::size_t c=0;
    for( ; i!=cv.end(); ++i, ++c) { }
    BOOST_CHECK(c==12);
    BOOST_CHECK(*i==44);

    std::advance(i, cv.size()-1);
    BOOST_CHECK(*i==32767);
    BOOST_CHECK(*++i==44);
    ++i;
    std::advance(i, 3*cv.size());
    BOOST_CHECK((*i)==(255-44));
}

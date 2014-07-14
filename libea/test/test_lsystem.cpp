/* test_lsystem.cpp
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
#include <algorithm>
#include "test.h"
#include <ea/lsystem.h>

BOOST_AUTO_TEST_CASE(test_lsystem_algae) {
    typedef lsystem<char> lsys_type;
    lsys_type L;
    
    L.symbol('A')
    .symbol('B')
    .axiom(L.string('A'))
    .rule('A', L.splitc("AB"))
    .rule('B', L.string('A'));
    
    lsys_type::string_type s = L.exec_n(7);
    const std::string t("ABAABABAABAABABAABABAABAABABAABAAB");
    BOOST_CHECK(std::equal(s.begin(), s.end(), t.begin()));
}

BOOST_AUTO_TEST_CASE(test_lsystem_turtle) {
    typedef lsystem<char> lsys_type;
    lsys_type L;
    
    L.symbol('0')
    .symbol('1')
    .symbol('[')
    .symbol(']')
    .axiom(L.string('0'))
    .rule('1', L.splitc("11"))
    .rule('0', L.splitc("1[0]0"));
    
    lsys_type::string_type s = L.exec_n(3);
    const std::string t("1111[11[1[0]0]1[0]0]11[1[0]0]1[0]0");
    BOOST_CHECK(std::equal(s.begin(), s.end(), t.begin()));
}

BOOST_AUTO_TEST_CASE(test_lsystem_koch) {
    typedef lsystem_turtle2<python_grid2> LSystem;
    LSystem L;
    L.axiom(L.string('F')).rule('F', L.splitc("F+F-F-F+F"));
    
    L.context()
    .origin(0,0)
    .angle(90)
    .heading(1,0)
    .step_magnitude(1.0);
    
    LSystem::string_type s = L.exec_n(2);
    const std::string t("F+F-F-F+F+F+F-F-F+F-F+F-F-F+F-F+F-F-F+F+F+F-F-F+F");
    BOOST_CHECK(std::equal(s.begin(), s.end(), t.begin()));
    
    python_grid2 g("koch.py");
    L.draw(g,s);
}

BOOST_AUTO_TEST_CASE(test_lsystem_dragon) {
    typedef lsystem_turtle2<python_grid2> LSystem;
    LSystem L;
    L.axiom(L.splitc("FX"))
    .rule('X', L.splitc("X+YF"))
    .rule('Y', L.splitc("FX-Y"));
    
    L.context()
    .origin(0,0)
    .angle(90)
    .heading(1,0);
    
    python_grid2 g("dragon.py");
    L.draw(g,10);
}

BOOST_AUTO_TEST_CASE(test_lsystem_plant) {
    typedef lsystem_turtle2<python_grid2, pointS> LSystem;
    LSystem L;
    L.axiom(L.splitc("X"))
    .rule('F', L.splitc("FF"))
    .rule('X', L.splitc("F-[[X]+X]+F[+FX]-X"));
    
    L.context()
    .origin(0,0)
    .angle(-25)
    .heading(1,2);
    
    python_grid2 g("plant.py");
    L.draw(g,6);
}


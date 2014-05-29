/* translation.h
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
#ifndef _EA_TRANSLATE_H_
#define _EA_TRANSLATE_H_

namespace ealib {
    
    /* WARNING: This is not yet well-developed. */

    /*! Translate the given genome (which must be iterable).
     */
    template <typename Genome, typename StartCodon, typename Translator, typename Phenotype>
    void translate_genome(const Genome& g, const StartCodon& start, const Translator& translate, Phenotype& p) {
        for(typename Genome::const_iterator f=g.begin(); f!=g.end(); ++f) {
            if(start(f)) {
                translate(f,p);
            }
        }
    }

} // ealib

#endif

/** @file causalization_json.hpp

 @brief <b>JSON generator of SBG causalization result</b>

 This json generator is not part of the library per se, but is included here to
 analyse faster the output of the causalization of Modelica models using the SBG
 approach. In the future it should belong to ModelicaCC instead.

 <hr>

 This file is part of Set--Based Graph Library.

 SBG Library is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 SBG Library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with SBG Library.  If not, see <http://www.gnu.org/licenses/>.

 ******************************************************************************/

#ifndef MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_BUILDERS_CAUSALIZATION_JSON_HPP_
#define MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_BUILDERS_CAUSALIZATION_JSON_HPP_

#include <sbg/pw_map.hpp>
#include <sbg/set.hpp>

namespace Modelica {

namespace Causalize {

class CausalizationResult {
public:
  CausalizationResult(SBG::LIB::Set horizontal_sorting
    , SBG::LIB::PWMap algebraic_loops
    , SBG::LIB::Set mfvs
    , SBG::LIB::PWMap vertical_sorting);

  const SBG::LIB::Set& horizontal_sorting() const;
  const SBG::LIB::PWMap& algebraic_loops() const;
  const SBG::LIB::Set& mfvs() const;
  const SBG::LIB::PWMap& vertical_sorting() const;

private:
  SBG::LIB::Set _horizontal_sorting;
  SBG::LIB::PWMap _algebraic_loops;
  SBG::LIB::Set _mfvs;
  SBG::LIB::PWMap _vertical_sorting;
};

void toJSON(const CausalizationResult& causalized);

} // namespace Causalize

} // namespace Modelica

#endif // MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_BUILDERS_CAUSALIZATION_JSON_HPP_

/*******************************************************************************

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

#include "causalize/sbg_implementation/builders/causalization_json.hpp"

#include <rapidjson/document.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/prettywriter.h>

namespace Modelica {

namespace Causalize {

////////////////////////////////////////////////////////////////////////////////
// Causalization result --------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

CausalizationResult::CausalizationResult(SBG::LIB::Set horizontal_sorting 
  , SBG::LIB::PWMap algebraic_loops
  , SBG::LIB::Set mfvs
  , SBG::LIB::PWMap vertical_sorting) : _horizontal_sorting(horizontal_sorting)
    , _algebraic_loops(algebraic_loops)
    , _mfvs(mfvs)
    , _vertical_sorting(vertical_sorting) {}

const SBG::LIB::Set& CausalizationResult::horizontal_sorting() const
{
  return _horizontal_sorting;
}

const SBG::LIB::PWMap& CausalizationResult::algebraic_loops() const
{
  return _algebraic_loops;
}

const SBG::LIB::Set& CausalizationResult::mfvs() const { return _mfvs; }

const SBG::LIB::PWMap& CausalizationResult::vertical_sorting() const
{
  return _vertical_sorting;
}

////////////////////////////////////////////////////////////////////////////////
// Build JSON file -------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

void toJSON(const CausalizationResult& causalized)
{
  // Initialize rapidJSON
  rapidjson::Document document;
  document.SetObject();
  rapidjson::Document::AllocatorType& alloc = document.GetAllocator();

  // Save causalization results
  document.AddMember("horizontal_sorting"
    , causalized.horizontal_sorting().toJSON(alloc), alloc);

  document.AddMember("algebraic_loops"
    , causalized.algebraic_loops().toJSON(alloc), alloc);

  document.AddMember("mfvs", causalized.mfvs().toJSON(alloc), alloc);

  document.AddMember("vertical_sorting"
    , causalized.vertical_sorting().toJSON(alloc), alloc);

  // Write file with rapidJSON
  FILE *fp = fopen("output.json", "w");
  char write_buffer[65536];
  rapidjson::FileWriteStream os(fp, write_buffer, sizeof(write_buffer));
  rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
  rapidjson::PrettyFormatOptions opt = rapidjson::kFormatSingleLineArray;
  writer.SetFormatOptions(opt);
  document.Accept(writer);

  fclose(fp);
}

} // namespace Causalize

} // namespace Modelica

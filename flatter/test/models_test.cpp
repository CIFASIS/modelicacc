/*****************************************************************************

    This file is part of Modelica C Compiler.

    Modelica C Compiler is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Modelica C Compiler is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Modelica C Compiler.  If not, see <http://www.gnu.org/licenses/>.

******************************************************************************/

#include <gtest/gtest.h>

#include <cstdlib>
#include <fstream>

/// @file models_test.cpp
///

/// @addtogroup MCCITests
/// @{

class ITests : public testing::TestWithParam<const char*> {
};

TEST_P(ITests, GenerateCode)
{
  const std::string NAME = GetParam();
  std::cout << "Testing model: " << NAME << std::endl;
  const std::string FLATTER = "../../bin/flatter ";
  const std::string ARGS = "../../test/mccprograms/" + NAME + ".mo -d -g ./test_data/" + NAME + ".dot" ;
  const std::string RESULT_FILE = "./test_data/" + NAME + ".passed";
  const std::string TEST_CMD = "./test_results.sh " + NAME;
  const std::string FLATTER_CMD = FLATTER + ARGS + " > ./test_data/" + NAME + ".mo";

  std::system(FLATTER_CMD.c_str());
  std::system(TEST_CMD.c_str());

  std::ifstream result(RESULT_FILE.c_str());
  EXPECT_TRUE(result.good());
}

const char* models[] = {"SBGraph1", "SBGraph2", "SBGraph3", "SBGraph4"};

INSTANTIATE_TEST_SUITE_P(Models, ITests, testing::ValuesIn(models));

/// @}

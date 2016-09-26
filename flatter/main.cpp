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

#include <flatter/connectors.h>
#include <mmo/mmo_class.h>
#include <mmo/mmo_tree.h>
#include <util/table.h>
#include <flatter/flatter.h>
#include <iostream>
#include <parser/parser.h>
#include <boost/variant/get.hpp>
#include <flatter/class_finder.h>
#include <unistd.h>

int main(int argc, char ** argv)
{
	using namespace std;
	using namespace Modelica::AST;
	using namespace Modelica;
	using namespace boost;
	
	bool ret;
	char * className = NULL, * filename = NULL, * out = NULL;
	char opt; 
	int debug = 0;
	std::ofstream outputFile;
	
	while ((opt = getopt(argc, argv, "o:i:c:g:d")) != -1) {
		switch (opt) {
		case 'o':
			out = optarg;
			break;
		case 'g':
			filename = optarg;
			break;	
		case 'd':
			debug = 1;	
		case 'c':
			className = optarg;
			break;	
		}
	}
	
	if (out) outputFile.open(out);
	/*if (className==NULL) {
		cerr << "Please, specify Class to flat. Option -c" << endl;
		exit(-1);
	}*/

	StoredDef sd;
  if (argv[optind]!=NULL)
    sd=Parser::ParseFile(argv[optind],ret);
  else
    sd=Parser::ParseFile("",ret);


	if (ret) {
		MMO_Tree mt;
		MMO_Class mmo = mt.create(sd);

		Flatter f = Flatter();
		Connectors co = Connectors(mmo);
    if (className == NULL) { // if no specified, flat last class
      std::string classToFlat = ::className(sd.classes().back());
      className = strdup(classToFlat.c_str());
    }
		if (className != NULL) {
			if (debug) std::cerr << "Searching for class " << (className?className:"NULL") << std::endl;
			ClassFinder re = ClassFinder();
			OptTypeDefinition m = re.resolveType(mmo,className);
			if (m) {
				typeDefinition td = m.get();
				Type::Type t_final = get<1>(td);
				if ( is<Type::Class>(t_final))
					mmo =  *(boost::get<Type::Class>(t_final).clase());
			}
		}

		if (debug) {
			std::cerr << "Class to  Flat: " << endl;
			std::cerr <<  mmo  << std::endl;
			std::cerr <<  " - - - - - - - - - - - - - - - - - - - - - - - - "  << std::endl << std::endl;
		}

		f.Flat(mmo,false,true);
		if (debug) {
			std::cerr << "First Flatter: " << endl;
			std::cerr <<  mmo  << std::endl;
			std::cerr <<  " - - - - - - - - - - - - - - - - - - - - - - - - "  << std::endl << std::endl;
		}

		co.resolve(mmo);
		if (debug) {
			std::cerr <<  " - - - - - - - - - - - - - - - - - - - - - - - - "  << std::endl;
			co.Debug(filename);
			std::cerr <<  " - - - - - - - - - - - - - - - - - - - - - - - - "  << std::endl;
		}

		//f.Flat(mmo,true,true);
		f.removeConnectorVar(mmo);
		if (debug) std::cerr << "Final Result: " << endl;
		if (out) {
			if (debug) std::cout <<  mmo << std::endl;
			outputFile <<  mmo << std::endl;
			outputFile.close();
		} else std::cout <<  mmo << std::endl;
	} else
		std::cout << "Error parser" <<  std::endl;

	return 0;
}



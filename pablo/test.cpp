// map < Vertex , map < MDI, T > >
#include <iostream>
#include <map>
#include <vector>
using namespace std;

typedef int Vertex;
typedef int MDI;
typedef int Value;
typedef map<MDI, Value> ListValues;
typedef map<Vertex, ListValues > Struct;

bool interseca (MDI a, MDI b){ // Para que compile
	return true;
}
MDI intersection (MDI a, MDI b){ // Para que compile
	return a;
}
MDI resta (MDI a, MDI b){
	return a;
}
bool empty (MDI a){
	return false;
}


vector <MDI> buscar_mdi (ListValues lv, MDI mdi){
	vector <MDI> rta;
	for (auto par : lv){
		if (interseca (par.first, mdi)){
			rta.push_back (intersection (par.first, mdi));
		}
	}
	return rta;
}

vector <MDI> buscar_value (ListValues lv, Value v){
	vector <MDI> rta;
	for (auto par : lv){
		if (par.second == v){
			rta.push_back (par.first);
		}
	}
	return rta;
}

ListValues set_mdi (ListValues lv, MDI mdi, Value v){
	ListValues rta;
	rta[mdi] = v;
	for (auto par : lv){
		if (interseca (par.first, mdi)){
			MDI rest = resta (par.first, mdi);
			if (!empty(rest))
				rta[rest] = par.second;
		}
		else{
			rta[par.first] = par.second;
		}
	}
	return rta;
}


int main(){
	Vertex a = 3;
	
	
	return 0;
}

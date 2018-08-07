#include"chaos.h"

Chaos* Chaos::instance = new Chaos();
Chaos* Chaos::getInstance(){
	static bool firflag=true;
	if(firflag){
		instance->packNum=1000000001;
		firflag=false;
	}
	return instance;
}



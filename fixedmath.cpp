#include "fixedmath.h"

unsigned u32atoi(const char*src){
	unsigned ans=0;
	int i=0;
	while((src[i]>='0')&&(src[i]<='9')){
		ans*=10;
		ans+=(src[i]-'0');
		i++;
	}
	return ans;
}
unsigned u32atox(const char* src){
	unsigned ans=0;
	int i=0;
	while(1){
		if((src[i]>='0')&&(src[i]<='9')){
			ans*=16;
			ans+=(src[i]-'0');
			i++;
		}else if((src[i]>='a')&&(src[i]<='f')){
			ans*=16;
			ans+=(10+src[i]-'a');
			i++;
		}else if((src[i]>='A')&&(src[i]<='F')){
			ans*=16;
			ans+=(10+src[i]-'A');
			i++;
		}else{
			return ans;
		}
	}
}



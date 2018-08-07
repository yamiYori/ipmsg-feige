#ifndef __CHAOS_H_
#define __CHAOS_H_
#include<atomic>
using std::atomic;
class Chaos{
	//singleton
	public:
		static Chaos* getInstance();
	private:
		Chaos()=default;
		static Chaos* instance;

	//members
	public:
		bool udpListen=false;
		bool tcpListen=false;
		bool programOK=true;
		atomic<unsigned>packNum;

		int udpsocket;

};



#endif

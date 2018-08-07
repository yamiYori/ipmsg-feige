obj = main.o chaos.o udpReceiver.o tcpReceiver.o fixedmath.o packMaker.o usermanager.o fileManager.o msgManager.o
target = main
INC = -I./include
DFN =             
CXXFLAGS = -std=gnu++11 -lpthread $(DFN)

$(target):$(obj)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(INC)
main.o:main.cpp udpReceiver.o chaos.o packMaker.o usermanager.o
	$(CXX) -c $< $(CXXFLAGS) $(INC)
udpReceiver.o:udpReceiver.cpp chaos.o fixedmath.o packMaker.o usermanager.o fixedmath.o fileManager.o msgManager.o
	$(CXX) -c $< $(CXXFLAGS) $(INC)
tcpReceiver.o:tcpReceiver.cpp chaos.o fixedmath.o packMaker.o usermanager.o fixedmath.o fileManager.o msgManager.o
chaos.o:chaos.cpp chaos.h
	$(CXX) -c $< $(CXXFLAGS) $(INC)
fixedmath.o:fixedmath.cpp fixedmath.h
	$(CXX) -c $< $(CXXFLAGS) $(INC)
packMaker.o:packMaker.cpp packMaker.h usermanager.o chaos.o
	$(CXX) -c $< $(CXXFLAGS) $(INC)
usermanager.o:usermanager.cpp usermanager.h
	$(CXX) -c $< $(CXXFLAGS) $(INC)
fileManager.o:fileManager.cpp fileManager.h
	$(CXX) -c $< $(CXXFLAGS) $(INC)
msgManager.o:msgManager.cpp msgManager.h
	$(CXX) -c $< $(CXXFLAGS) $(INC)

clean:
	rm $(obj) $(target)

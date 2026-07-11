CXX = c++
CXXFLAGS = -Wall -fpermissive -fms-extensions -g
CPPFLAGS = -DUNIX -DLINUX -I.
LDLIBS = -lpthread

SOURCES_CPP = \
	Core/DXPSHelper.cpp \
	Core/UnixCompat.cpp \
	Core/Socket/DXPSSock.cpp \
	Core/Reflect/DXPSReflection.cpp \
	Core/Reflect/DXPSReflect.cpp \
	Core/Server/DXPSServer.cpp \
	DXPSShaderCompiler.cpp

DXPSShaderCompiler: $(patsubst %.cpp,%.o,$(SOURCES_CPP))
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

clean:
	rm -f DXPSShaderCompiler
	rm -f $(patsubst %.cpp,%.o,$(SOURCES_CPP))


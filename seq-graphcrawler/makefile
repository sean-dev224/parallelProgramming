CXXFLAGS=-I ./rapidjson/include
LDFLAGS=-lcurl
LD=g++

seq-graphcrawler.out: seq-graphcrawler.o
		$(LD) -o $@ $< $(LDFLAGS)

seq-graphcrawler.o: seq-graphcrawler.cpp
		$(CXX) $(CXXFLAGS) -c -o $@ $<


rapidjson_test.out: rapidjson_test.o
		$(LD) -o $@ $<

rapidjson_test.o: rapidjson_test.cpp
		$(CXX) $(CXXFLAGS) -c -o $@ $<


curl_test.out: curl_test.o
		$(LD) -o $@ $< $(LDFLAGS)

curl_test.o: curl_test.cpp
		$(CXX) $(CXXFLAGS) -c -o $@ $<


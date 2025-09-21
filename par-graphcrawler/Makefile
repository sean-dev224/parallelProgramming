#CXXFLAGS=-I path/to/rapidjson
LDFLAGS=-lcurl
LD=g++
CC=g++

all: level_client par-graphcrawler

level_client: level_client.o
	$(LD) $< -o $@ $(LDFLAGS)

par-graphcrawler: par-graphcrawler.o
	$(LD) $< -o $@ $(LDFLAGS)



clean:
	-rm level_client level_client.o par-graphcrawler par-graphcrawler.o

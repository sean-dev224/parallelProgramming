#CXXFLAGS=-I path/to/rapidjson
LDFLAGS=-lcurl
LD=g++
CC=g++

all: queue-graphcrawler


queue-graphcrawler: queue-graphcrawler.o
	$(LD) $< -o $@ $(LDFLAGS)



clean:
	-rm queue-graphcrawler queue-graphcrawler.o

.PHONY: all

all: packet

packet:
	g++ -o packet_example Packet_Transmmit.cpp

clean:
	rm -rf packet_example
.PHONY: all

all: IPv4addr, Dname2Ipaddr

IPv4addr: addr_transform_IPv4.cpp
	g++ -o IPv4addr addr_transform_IPv4.cpp

Dname2Ipaddr:
	g++ -o Dname2Ipaddr dname_to_ipaddr.cpp

clean:
	rm -f IPv4 addr Dname2Ipaddr
/**
 * @file Packet_Transmmit.cpp
 * @brief Demonstrates basic network packet transmission using TCP, UDP, and ICMP protocols in C++.
 *
 * This file provides three functions for network connectivity testing:
 * - TCP(): Attempts to establish a TCP connection to a specified IP address and port, measuring connection time.
 * - UDP(): Sends a UDP packet to a specified IP address and port.
 * - ICMP(): Sends an ICMP Echo Request (ping) to a specified IP address and waits for a reply, measuring round-trip time.
 *
 * Each function prints the result of its operation to the console.
 * The code uses standard Linux socket APIs and requires root privileges for ICMP (raw socket) operations.
 * Change the target IP and port in each function as needed for your testing scenario.
 */
 
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h> // For icmphdr and ICMP_ECHO
#include <arpa/inet.h>
#include <unistd.h>
#include <chrono>
#include <sys/time.h> // For struct timeval
#include <fcntl.h> // For fcntl, F_GETFL, F_SETFL, O_NONBLOCK

// Global parameters for destination IP and port
// These are used in all functions to set up the destination address
// dest_ip: Target IP address for all transmissions
// dest_port: Target port number for TCP and UDP
const char* dest_ip = "10.17.74.158"; // Target IP address
const int dest_port = 12345; // Target port number

// -----------------------------------------------------------------------------
// Checksum Function
// -----------------------------------------------------------------------------
// Calculates the Internet checksum for ICMP packets, ensuring data integrity.
// Used by ICMP() to build valid packets.
unsigned short checksum(void* b, int len) {
	unsigned short* buf = (unsigned short*)b;
	unsigned int sum = 0;
	unsigned short result;

	// Sum all 16-bit words
	for (sum = 0; len > 1; len -= 2)
		sum += *buf++;
		printf("Intermediate sum: %u\n", sum);
	// If there's a leftover byte, add it
	if (len == 1)
		sum += *(unsigned char*)buf;
	// Fold 32-bit sum to 16 bits
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	printf("Final sum before complement: %u\n", sum);
	result = ~sum;
	printf("Checksum result: %u\n", result);
	return result;
}

// -----------------------------------------------------------------------------
// TCP Function
// -----------------------------------------------------------------------------
// Creates a TCP socket, sets up the destination address, attempts to connect,
// measures connection time, prints result, and closes the socket.
int TCP() {

	// Create a TCP socket (SOCK_STREAM)
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		std::cerr << "TCP socket creation failed" << std::endl;
		return 1;
	}

	// Set socket to non-blocking mode
	int flags = fcntl(sock, F_GETFL, 0);
	if (flags < 0) flags = 0;
	fcntl(sock, F_SETFL, flags | O_NONBLOCK);

	// Set up the destination address structure
	sockaddr_in dest_addr;
	std::memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.sin_family = AF_INET; // IPv4
	dest_addr.sin_port = htons(dest_port); // Port in network byte order
	printf("dest_addr family: %d, port: %d\n", dest_addr.sin_family, ntohs(dest_addr.sin_port));
	// Convert IP address from text to binary form
	if (inet_pton(AF_INET, dest_ip, &dest_addr.sin_addr) <= 0) {
		std::cerr << "Invalid address" << std::endl;
		close(sock);
		return 1;
	}

	// Attempt to connect to the server (non-blocking)
	auto start = std::chrono::steady_clock::now();
	int result = connect(sock, (sockaddr*)&dest_addr, sizeof(dest_addr));
	if (result < 0) {
		if (errno == EINPROGRESS) {
			// Connection in progress, wait for it to complete or timeout
			fd_set writefds;
			FD_ZERO(&writefds);
			FD_SET(sock, &writefds);
			struct timeval tv;
			tv.tv_sec = 1; // 1 second timeout
			tv.tv_usec = 0;
			int sel = select(sock + 1, NULL, &writefds, NULL, &tv);
			auto end = std::chrono::steady_clock::now();
			if (sel > 0 && FD_ISSET(sock, &writefds)) {
				int so_error = 0;
				socklen_t len = sizeof(so_error);
				getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &len);
				if (so_error == 0) {
					double ms = std::chrono::duration<double, std::milli>(end - start).count();
					std::cout << "TCP connection to " << dest_ip << ":" << dest_port << " succeeded in " << ms << " ms" << std::endl;
				} else {
					std::cerr << "TCP connection failed (SO_ERROR=" << so_error << ")" << std::endl;
					close(sock);
					return 1;
				}
			} else {
				std::cerr << "TCP connection timed out" << std::endl;
				close(sock);
				return 1;
			}
		} else {
			std::cerr << "TCP connection failed immediately" << std::endl;
			close(sock);
			return 1;
		}
	} else {
		auto end = std::chrono::steady_clock::now();
		double ms = std::chrono::duration<double, std::milli>(end - start).count();
		std::cout << "TCP connection to " << dest_ip << ":" << dest_port << " succeeded in " << ms << " ms" << std::endl;
	}

	// Restore socket to blocking mode
	fcntl(sock, F_SETFL, flags);
	// Close the socket
	close(sock);
	return 0;
}

// -----------------------------------------------------------------------------
// ICMP Function
// -----------------------------------------------------------------------------
// Creates a raw socket for ICMP (requires root privileges), sets up the destination,
// builds and sends an ICMP Echo Request (ping), waits for reply, measures round-trip time,
// prints result, and closes the socket.
int ICMP() {
	// Create a raw socket for ICMP (requires root privileges)
	int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sock < 0) {
		std::cerr << "Raw socket creation failed. Try running as root." << std::endl;
		return 1;
	}

	// Set up the destination address structure
	sockaddr_in dest_addr;
	std::memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.sin_family = AF_INET;
	// Convert IP address from text to binary form
	if (inet_pton(AF_INET, dest_ip, &dest_addr.sin_addr) <= 0) {
		std::cerr << "Invalid address" << std::endl;
		close(sock);
		return 1;
	}

	// Build ICMP Echo Request packet
	char packet[64]; // Buffer for ICMP packet
	std::memset(packet, 0, sizeof(packet));
	icmphdr* icmp = (icmphdr*)packet; // ICMP header pointer
	icmp->type = ICMP_ECHO; // Echo request
	icmp->code = 0;
	icmp->un.echo.id = getpid() & 0xFFFF; // Identifier (usually process ID)
	icmp->un.echo.sequence = 1; // Sequence number
	// Fill the rest of the packet with dummy data
	std::memset(packet + sizeof(icmphdr), 'A', sizeof(packet) - sizeof(icmphdr));
	icmp->checksum = 0;
	icmp->checksum = checksum(packet, sizeof(packet)); // Calculate checksum

	// Send ICMP Echo Request and measure time
	auto start = std::chrono::steady_clock::now();
	ssize_t sent = sendto(sock, packet, sizeof(packet), 0,
						  (sockaddr*)&dest_addr, sizeof(dest_addr));
	if (sent < 0) {
		std::cerr << "Send failed" << std::endl;
		close(sock);
		return 1;
	}

	// Receive ICMP Echo Reply
	char recv_buf[1024]; // Buffer for reply
	sockaddr_in recv_addr;
	socklen_t addr_len = sizeof(recv_addr);
	printf("addr_len %d\n", addr_len);
	ssize_t recv_len = recvfrom(sock, recv_buf, sizeof(recv_buf), 0,
								(sockaddr*)&recv_addr, &addr_len);
	auto end = std::chrono::steady_clock::now();
	printf("recv_len %ld\n", recv_len);
	if (recv_len < 0) {
		std::cerr << "Receive failed" << std::endl;
	} else {
		double ms = std::chrono::duration<double, std::milli>(end - start).count();
		std::cout << "Received ICMP reply from " << dest_ip << " in " << ms << " ms" << std::endl;
	}
	// Set a receive timeout of 1 second for the socket
	struct timeval tm;
	tm.tv_sec = 1; // seconds
	tm.tv_usec = 0; // microseconds
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tm, sizeof(tm)) < 0) {
		std::cerr << "Failed to set socket receive timeout" << std::endl;
		close(sock);
		return 1;
	}
	// Close the socket
	close(sock);
	return 0;
}

// -----------------------------------------------------------------------------
// UDP Function
// -----------------------------------------------------------------------------
// Creates a UDP socket, sets up the destination address, sends a message,
// prints result, and closes the socket.
int UDP() {

	// Message to send
	const char* message = "Hello, UDP!";

	// Create a UDP socket (SOCK_DGRAM)
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		std::cerr << "Socket creation failed" << std::endl;
		return 1;
	}

	// Set up the destination address structure
	sockaddr_in dest_addr;
	std::memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.sin_family = AF_INET; // IPv4
	dest_addr.sin_port = htons(dest_port); // Port in network byte order
	printf("dest_addr family: %d, port: %d\n", dest_addr.sin_family, ntohs(dest_addr.sin_port));
	// Convert IP address from text to binary form
	if (inet_pton(AF_INET, dest_ip, &dest_addr.sin_addr) <= 0) {
		std::cerr << "Invalid address" << std::endl;
		close(sock);
		return 1;
	}

	// Send the UDP packet to the destination
	ssize_t sent = sendto(sock, message, std::strlen(message), 0,
						  (sockaddr*)&dest_addr, sizeof(dest_addr));
	printf("sent %ld\n", sent);
	if (sent < 0) {
		std::cerr << "Send failed" << std::endl;
	} else {
		std::cout << "Packet sent to " << dest_ip << ":" << dest_port << std::endl;
	}
	// Set a receive timeout of 1 second for the socket
	struct timeval tm;
	tm.tv_sec = 1; // seconds
	tm.tv_usec = 0; // microseconds
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tm, sizeof(tm)) < 0) {
		std::cerr << "Failed to set socket receive timeout" << std::endl;
		close(sock);
		return 1;
	}
	// Close the socket
	close(sock);
	return 0;
}

// -----------------------------------------------------------------------------
// main()
// -----------------------------------------------------------------------------
// Calls the ICMP() function to perform a ping test.
// You can call TCP() or UDP() here as needed for other tests.
int main(){
	printf("Target IP: %s, Target Port: %d\n", dest_ip, dest_port);
	printf("\n");
	printf("ICMP Testing\n");
	ICMP();
	printf("\n");
	printf("TCP Testing\n");
	TCP();
	printf("\n");
	printf("UDP Testing\n");
	UDP();
	return 0;    
}
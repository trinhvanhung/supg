//
// Created by chungphb on 21/5/21.
//

#include <iostream>
#include <string>
#include <cstring>
#include <cctype>
#include <algorithm>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <bitset>
constexpr uint16_t SERVER_PORT = 1700;
constexpr size_t GATEWAY_MAC_LEN = 8;
constexpr size_t ACK_BUFFER_SIZE = 1024;
const char GATEWAY_MAC[GATEWAY_MAC_LEN] = {0x02, 0x05, 0x01, 0x00, 0x01, 0x09, 0x09, 0x06};

void push_data(int& sock_fd, sockaddr_in& server_addr, int max,int start) {
    for(int j=0; j < max; j++){
        // Create a Semtech UDP packet
        std::string packet(GATEWAY_MAC_LEN + 4, 0);
        packet[0] = 0x02;
        // std::bitset<64> x(start+j);
        // std::bitset<64> x1(0xff00 & (start+j) > 8);
        // std::cout << x << " " << x1  <<std::endl;
        packet[1] = (unsigned int)(0xff00 & (start+j)) >> 8 ;
        packet[2] = (unsigned int)(0xff & (start+j));
        packet[3] = 0x00;
        for (size_t i = 0; i < GATEWAY_MAC_LEN; ++i) {
            packet[i + 4] = GATEWAY_MAC[i];
        }
        std::string payload = R"({
            "rxpk": [
                {
                    "time":"2013-03-31T16:21:17.530974Z",
                    "tmst":3512348514,
                    "chan":9,
                    "rfch":1,
                    "freq":869.1,
                    "stat":1,
                    "modu":"FSK",
                    "datr":50000,
                    "rssi":-75,
                    "size":16,
                    "data":"VEVTVF9QQUNLRVRfMTIzNA=="
                }
            ]
        })";
        payload.erase(std::remove_if(payload.begin(), payload.end(), ::isspace), payload.end());
        packet += payload;

        // Send packet
        sendto(sock_fd, packet.c_str(), packet.length(), MSG_CONFIRM, (const sockaddr*)&server_addr, sizeof(server_addr));
        std::cout << "Send packet: " << packet << std::endl;

        // sleep(5);
        std::cout << "Receive!" << std::endl;
        // Receive ACK
        size_t len, addr_len;
        len=0;addr_len=0;
        char ack[ACK_BUFFER_SIZE];
        len = recvfrom(sock_fd, ack, ACK_BUFFER_SIZE, MSG_WAITALL, (sockaddr*)&server_addr, (socklen_t*)&addr_len);
        //std::cout << "Receive! \t!" << len << std::endl;
        std::cout << "Sent!" << (int)ack[0] << " " << (unsigned int)packet[1] << " " << (unsigned int)(packet[2] & 0xff) << " " << (int)ack[3] <<std::endl;
        std::cout << "Receive!" << (int)ack[0] << " " << (unsigned int)ack[1] << " " << (unsigned int)(ack[2] & 0xff) << " " << (int)ack[3];
        ack[len] = '\0';
        std::cout << "Receive ACK: " << ack << std::endl;
    }
}

int main() {
    // Create socket file descriptor
    int sock_fd;
    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "Failed to create socket file descriptor" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Update server information
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    //server_addr.sin_addr.s_addr = INADDR_ANY;
    inet_aton("116.97.245.226", &server_addr.sin_addr);

    int max = 10000;
    int start = 255;
    
    std::cout << "Max " <<max  <<  " " <<sizeof(max) << std::endl;
    // Push data
    push_data(sock_fd, server_addr,max,start);

    // Close socket file descriptor
    close(sock_fd);
    return 0;
}

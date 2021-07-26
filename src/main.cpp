
// You need to link Ws2_32 library

#include <iostream>
#include <WinSock2.h>
#include <ws2tcpip.h>

#include "events.h"
#include "bytesteam.h"

enum e_query_type : uint8_t
{
    QUERYTYPE_SERVERINFO = 'i',
    QUERYTYPE_SERVERRULES = 'r',
    QUERYTYPE_SERVERPING = 'p',
    QUERYTYPE_PLAYERLIST = 'c'
};

struct s_query
{
    const char samp_bytes[4];
    const char server_ip[4];
    const char server_port[2];
    const char packet_type;
};

class c_server
{
    SOCKET m_socket;
public:
    c_event<std::string, c_bytestream &> server_info;
    c_event<std::string, c_bytestream &> server_rules;
    c_event<std::string, c_bytestream &> player_list;
    c_event<std::string, c_bytestream &> server_ping;

    void init(u_short a_port)
    {
        WSADATA wsa_data;
        WSAStartup(MAKEWORD(2, 2), &wsa_data);
        
        sockaddr_in addr;
        ZeroMemory(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(a_port);

        m_socket = ::socket(addr.sin_family, SOCK_DGRAM, 0);

        ::bind(m_socket, reinterpret_cast<sockaddr *>(&addr), sizeof addr);

        std::cout << ". Server started on port " << a_port << std::endl;
    }

    void monitor()
    {
        char buffer[512];
        sockaddr from_addr;
        int from_len, recv_len;

        recv_len = ::recvfrom(m_socket, buffer, sizeof buffer, 0, &from_addr, &from_len);
        if (recv_len > 0)
        {
            std::string result;
            std::stringstream ss(std::string(buffer, recv_len));

            c_bytestream bs;
            bs.set_stream(ss);

            s_query *query = reinterpret_cast<s_query *>(buffer);
            switch (query->packet_type)
            {
                case QUERYTYPE_SERVERINFO:
                    result = server_info.call(bs);
                    break;
                case QUERYTYPE_SERVERRULES:
                    result = server_rules.call(bs);
                    break;
                case QUERYTYPE_PLAYERLIST:
                    result = player_list.call(bs);
                    break;
                case QUERYTYPE_SERVERPING:
                    result = server_ping.call(bs);
                    break;
                default:
                    std::cout << ". > Received unknown packet." << std::endl;
            }

            if (!result.empty())
            {
                ::sendto(m_socket, result.c_str(), result.length(), 0, &from_addr, from_len);
                std::cout << ". < Response sended." << std::endl;
            }
        }
    }

    void shutdown()
    {
        closesocket(m_socket);
        WSACleanup();
    }
};

int main()
{
    c_server srv;
    srv.init(7777); // Init server on port 7777

    srv.server_info.set([&](c_bytestream &bs)
    {
        std::cout << ". > Received SERVERINFO packet!" << std::endl;

        bs.write_num<char>(0, 11);
        bs.write_num<short>(999);
        bs.write_num<short>(999);
        bs.write_str("host");
        bs.write_str("mode");
        bs.write_str("lang");

        return bs.get();
    });
    // You can manually add rest callbacks
    // e.g. srv.callback_name.set(...);

    while (1)
    {
        srv.monitor();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    srv.shutdown();

    std::system("pause");
    return 0;
}
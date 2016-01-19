#include "NetworkingSystem.hpp"

#include <string>
#include <crtdbg.h>
#include <MSTcpIP.h>

#include "PacketHeader.hpp"

#pragma comment(lib, "Ws2_32.lib")
#define UNUSED(x) (void)(x);

namespace Henry
{

	NetworkingSystem::NetworkingSystem(NetworkType type, const char* ipAddr, u_short port, int playerID, bool nonBlocking, bool echoServer)
	: m_type(type)
	, m_ipAddr(ipAddr)
	, m_port(port)
	, m_playerID(playerID)
	, m_echoServer(echoServer)
{
	WSAData wsaData;
	WORD DLLVSERION;
	DLLVSERION = MAKEWORD(2, 1);
	CheckAndReportError( WSAStartup(DLLVSERION, &wsaData) );

	m_addr.sin_addr.s_addr = inet_addr(m_ipAddr);
	m_addr.sin_family = AF_INET;
	m_addr.sin_port = htons(m_port);
	m_addrlen = sizeof(m_addr);

	m_maxPacketSizeInByte = 2048;
	m_tempBuffer = new char[m_maxPacketSizeInByte];
	m_orderedPacketID = 0;
	m_reliablePacketID = 0;
	m_nextProcessOrderedPacketID = 0;
	m_nextProcessReliablePacketID = 0;
	m_maxPacketProcessPerFrame = 100;
	m_heartbeatCounter = 1.0f;

	ZeroMemory(m_tempBuffer,m_maxPacketSizeInByte);

	u_long mode = nonBlocking ? 1 : 0;
	switch(m_type)
	{
// 	case TCP_CLIENT:
// 		m_connect = socket(AF_INET, SOCK_STREAM, NULL);
// 		connect(m_connect, (SOCKADDR*)&m_addr, sizeof(m_addr));
// 		CheckAndReportError( ioctlsocket(m_connect, FIONBIO, &mode) );
// 		break;
// 	case TCP_SERVER:
// 		m_listen = socket(AF_INET, SOCK_STREAM, NULL);
// 		CheckAndReportError( bind(m_listen, (SOCKADDR*)&m_addr, sizeof(m_addr)) );
// 		listen(m_listen, SOMAXCONN);
// 		CheckAndReportError( ioctlsocket(m_listen, FIONBIO, &mode) );
// 		break;
	case UDP_CLIENT:
		m_connect = socket(AF_INET, SOCK_DGRAM, NULL);
		connect(m_connect, (SOCKADDR*)&m_addr, sizeof(m_addr));
		CheckAndReportError( ioctlsocket(m_connect, FIONBIO, &mode) );
		break;
	case UDP_SERVER:
		m_listen = socket(AF_INET, SOCK_DGRAM, NULL);
		CheckAndReportError( bind(m_listen, (SOCKADDR*)&m_addr, sizeof(m_addr)) );
		CheckAndReportError( ioctlsocket(m_listen, FIONBIO, &mode) );
		break;
	case RAW_SOCKET:
		m_listen = socket(AF_INET, SOCK_RAW, IPPROTO_IP);
		CheckAndReportError(bind(m_listen, (SOCKADDR*)&m_addr, sizeof(m_addr)));
		//CheckAndReportError(ioctlsocket(m_listen, SIO_RCVALL, &mode));
		CheckAndReportError(WSAIoctl(m_listen, SIO_RCVALL, &mode, sizeof(mode), 0, 0, (LPDWORD)&mode, 0, 0));
	}
}


NetworkingSystem::~NetworkingSystem(void)
{
	delete m_tempBuffer;
	m_tempBuffer = nullptr;
}


void NetworkingSystem::CheckAndReportError(int returnCode)
{
	if(returnCode < 0)
	{
		std::string errorInfo;
		errorInfo = "Initialized failed with error : " + WSAGetLastError();
		MessageBoxA( NULL , errorInfo.c_str() , "Error Initialize Network" , MB_ICONERROR | MB_OK);
		exit(0);
	}
}


void NetworkingSystem::Update(float deltaSeconds)
{
	ZeroMemory(m_tempBuffer, m_maxPacketSizeInByte);
	ClearReceiveBuffer();

	switch(m_type)
	{
// 	case TCP_CLIENT:
// 		break;
// 	case TCP_SERVER:
// 		//SOCKADDR_IN clinetAddr;
// 		//SOCKET sConnect = accept(m_listen, (SOCKADDR*)&clinetAddr, &m_addrlen);
// 		break;
	case UDP_CLIENT:
		HeartBeat(deltaSeconds);
		RefreshSendListFromWaitProcessList();
		ReceiveMessage();
		SendAllMessages();
		break;
	case UDP_SERVER:
	{
		RefreshConnectStatus(deltaSeconds);
		ClientInfo clinet;
		int recvLength = recvfrom(m_listen, m_tempBuffer, m_maxPacketSizeInByte, NULL, (SOCKADDR*)&clinet.SocketAddr, &m_addrlen);
		while (recvLength > 0)
		{
			m_tempBuffer[recvLength] = NULL;
			std::string clientID(inet_ntoa(clinet.SocketAddr.sin_addr));
			clientID += "" + clinet.SocketAddr.sin_port;

			if (m_clinetMap.find(clientID) == m_clinetMap.end())
				m_clinetMap[clientID] = clinet;

			char* buffer = new char[recvLength];
			memcpy(buffer, m_tempBuffer, recvLength);
			m_receiveBuffer.push_back(buffer);

			if (m_echoServer)
				sendto(m_listen, m_tempBuffer, recvLength, NULL, (SOCKADDR*)&clinet.SocketAddr, m_addrlen);

			ZeroMemory(m_tempBuffer, m_maxPacketSizeInByte);
			recvLength = recvfrom(m_listen, m_tempBuffer, m_maxPacketSizeInByte, NULL, (SOCKADDR*)&clinet.SocketAddr, &m_addrlen);
		}
	}
		break;
	case RAW_SOCKET:
	{
		fopen_s(&m_file, "log.txt", "w");
		ClientInfo clinet;
		int recvLength = recvfrom(m_listen, m_tempBuffer, m_maxPacketSizeInByte, NULL, (SOCKADDR*)&clinet.SocketAddr, &m_addrlen);
		while (recvLength > 0)
		{
			m_tempBuffer[recvLength] = NULL;
			ip_hdr* ip = ((ip_hdr*)m_tempBuffer);
			char* ip_ptr = (char*)&ip->ip_destaddr;
			printf("from ip :");
			fprintf(m_file, "from ip :");
			for (int index = 0; index < 4; ++index)
			{
				if (ip_ptr)
				{ 
					printf("%d", (*ip_ptr));
					fprintf(m_file, "%d", (*ip_ptr));
					++ip_ptr;
				}
				
				if (index != 3)
				{
					printf(".");
					fprintf(m_file,".");
				}
				else
				{
					printf(" ,");
					fprintf(m_file," ,");
				}
			}

			if (ip->ip_protocol == 17) // udp
			{
				char* ptr = m_tempBuffer;
				ptr += sizeof(ip_hdr) * 4 + sizeof(udp_hdr);
				int size = recvLength - ip->ip_header_len * 4 - sizeof(udp_hdr);

				char* buffer = new char[size];
				memcpy(buffer, ptr, size);
				m_receiveBuffer.push_back(buffer);
				printf("size received : %d , raw data : ", size);
				fprintf(m_file,"size received : %d , raw data : ");
				for (int index = 0; index < recvLength; ++index)
				{
					printf("%c", buffer[index]);
					fprintf(m_file,"%c", buffer[index]);
				}
			}
			fprintf(m_file, "\n");
			printf("\n");
			ZeroMemory(m_tempBuffer, m_maxPacketSizeInByte);
			recvLength = recvfrom(m_listen, m_tempBuffer, m_maxPacketSizeInByte, NULL, (SOCKADDR*)&clinet.SocketAddr, &m_addrlen);
		}
		fclose(m_file);
	}
		break;
	}
}


void NetworkingSystem::ReceiveMessage()
{
	int recvLength = recvfrom(m_connect, m_tempBuffer, m_maxPacketSizeInByte, NULL, (SOCKADDR*)&m_addr, &m_addrlen);
	while (recvLength > 0)
	{
		PacketChannel channel = (PacketChannel)m_tempBuffer[0];
		int packetID = ExtractPacketIDFromPacket(m_tempBuffer);
		
		char* bufferPtr = m_tempBuffer;
		bufferPtr += (4 + sizeof(int));
		char* data = new char[recvLength - 4 - sizeof(int)];
		memcpy(data, bufferPtr, recvLength - 4 - sizeof(int));

		switch (channel)
		{
		case UNRELIABLE:
			if (data[0] == 1)	// Ack
				RemoveFromResendList(packetID);
			else
				m_processList.push_back(data);
			break;
		case ORDERED:
			if (packetID >= m_nextProcessOrderedPacketID )
			{
				m_nextProcessOrderedPacketID = packetID;
				m_processList.push_back(data);
			}
			break;
		case RELIABLE:
			if (packetID <= m_nextProcessReliablePacketID)
			{
				if (packetID == m_nextProcessReliablePacketID)
				{ 
					m_processList.push_back(data);
					++m_nextProcessReliablePacketID;
				}
				AcknowledgePacket ack;
				ack.info.channelID = UNRELIABLE;
				ack.info.packetID = packetID;
				ack.info.packetType = 1;
				ack.info.playerID = (char)m_playerID;
				
				char* packet = new char[sizeof(AcknowledgePacket)];
				memcpy(packet, ack.buffer, sizeof(AcknowledgePacket));
				PacketInfo pi;
				pi.buffer = packet;
				pi.size = sizeof(AcknowledgePacket);
				m_sendList.push_back(pi);
			}
			else if (packetID > m_nextProcessReliablePacketID)
			{
				char* originalData = new char[recvLength];
				memcpy(originalData, m_tempBuffer, recvLength);
				m_reliableProcessList.push(originalData);
				delete data;
			}
			break;
		default:
			break;
		}

		ZeroMemory(m_tempBuffer, m_maxPacketSizeInByte);
		recvLength = recvfrom(m_connect, m_tempBuffer, m_maxPacketSizeInByte, NULL, (SOCKADDR*)&m_addr, &m_addrlen);
	}
}


void NetworkingSystem::PushMessage(PacketChannel channel, const char* data, int size)
{
	char* packet = new char[size + 4 + sizeof(int)];
	char* bufferPtr = packet;
	
	*bufferPtr = (char)channel;
	bufferPtr += 4;

	switch (channel)
	{
	case UNRELIABLE:
		WriteIntegerToBuffer(bufferPtr, 0);
		break;
	case ORDERED:
		WriteIntegerToBuffer(bufferPtr, m_orderedPacketID);
		++m_orderedPacketID;
		break;
	case RELIABLE:
		WriteIntegerToBuffer(bufferPtr, m_reliablePacketID);
		++m_reliablePacketID;
		break;
	default:
		break;
	}

	memcpy(bufferPtr, data, size);
	PacketInfo pi;
	pi.buffer = packet;
	pi.size = size + 4 + sizeof(int);
	m_sendList.push_back(pi);

	if (channel == RELIABLE)
		m_resendList.push_back(pi);
}


void NetworkingSystem::RefreshConnectStatus(float deltaSeconds)
{
	std::map<std::string, ClientInfo>::iterator it = m_clinetMap.begin();
	while (it != m_clinetMap.end())
	{
		(it->second).TimeSinceLastUpdate += deltaSeconds;
		if ((it->second).TimeSinceLastUpdate > 5.0f)
		{
			m_clinetMap.erase(it->first);
			it = m_clinetMap.erase(it);
		}
		else
			++it;
	}
}


void NetworkingSystem::ClearReceiveBuffer()
{
	std::vector<char*>::iterator it = m_receiveBuffer.begin();
	while (it != m_receiveBuffer.end())
	{
		char* buffer = *it;
		it = m_receiveBuffer.erase(it);
		delete buffer;
		buffer = nullptr;
	}
}


void NetworkingSystem::WriteIntegerToBuffer(char*& buffer, int data)
{
	for (int index = 0; index < sizeof(int); ++index)
	{
		*buffer = (((unsigned char*)&data)[index]);
		++buffer;
	}
}


void NetworkingSystem::RefreshSendListFromWaitProcessList()
{
	while (m_processList.size() < m_maxPacketProcessPerFrame && !m_reliableProcessList.empty())
	{
		char* topPacket = m_reliableProcessList.top();
		int topPacketID = ExtractPacketIDFromPacket(topPacket);
		
		if (topPacketID > m_nextProcessReliablePacketID)
			break;
		
		if (topPacketID == m_nextProcessReliablePacketID)
		{
			m_reliableProcessList.pop();
			m_processList.push_back(topPacket);
			++m_nextProcessReliablePacketID;
		}

		if (topPacketID < m_nextProcessReliablePacketID)
		{
			delete topPacket;
			m_reliableProcessList.pop();
		}
	}
}


void NetworkingSystem::SendAllMessages()
{
	std::vector<PacketInfo>::iterator it = m_sendList.begin();
	while (it != m_sendList.end())
	{
		char* msg = (*it).buffer;
		int size = (*it).size;
		sendto(m_connect, msg, size, NULL, (SOCKADDR*)&m_addr, m_addrlen);
		it = m_sendList.erase(it);
		msg = nullptr;
	}
	
	for (size_t index = 0; index < m_resendList.size(); ++index)
		sendto(m_connect, m_resendList[index].buffer, m_resendList[index].size, NULL, (SOCKADDR*)&m_addr, m_addrlen);
}


void NetworkingSystem::RemoveFromResendList(int packetID)
{
	std::vector<PacketInfo>::iterator it = m_resendList.begin();
	while (it != m_resendList.end())
	{
		char* packetBuffer = (*it).buffer;
		if (ExtractPacketIDFromPacket(packetBuffer) <= packetID)
			it = m_resendList.erase(it);
		else
			++it;
	}
}


int NetworkingSystem::ExtractPacketIDFromPacket(char* packetBuffer)
{
	char* ptr = packetBuffer;
	ptr += 4;
	int packetID = (*(int*)ptr);
	return packetID;
}


void NetworkingSystem::HeartBeat(double deltaSeconds)
{
	m_heartbeatCounter += (float)deltaSeconds;
	if (m_heartbeatCounter >= 1.0f)
	{ 
		m_heartbeatCounter = 0.0f;
		HeartbeatPacket hp;
		hp.info.packetType = 0;
		hp.info.playerID = (char)m_playerID;
		PushMessage(UNRELIABLE, hp.buffer, sizeof(HeartbeatPacket));
	}
}


void NetworkingSystem::ResetOrderedAndReliablePacketID()
{
	m_nextProcessOrderedPacketID = 0;
	m_nextProcessReliablePacketID = 0;
}


};
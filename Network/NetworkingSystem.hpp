#pragma once

#ifndef NETWORKINGSYSTEM_HPP
#define NETWORKINGSYSTEM_HPP

#define WIN32_LEAN_AND_MEAN
#include <queue>
#include <map>
#include <WinSock2.h>
#include <vector>

namespace Henry
{

enum NetworkType{ TCP_SERVER , UDP_SERVER , TCP_CLIENT , UDP_CLIENT , RAW_SOCKET };
enum PacketChannel{ UNRELIABLE, ORDERED, RELIABLE };
struct ClientInfo
{
	ClientInfo(){ NextProcessOrderedPacketID = 0; NextProcessReliablePacketID = 0; TimeSinceLastUpdate = 0.0; };
	SOCKADDR_IN SocketAddr;
	int NextProcessOrderedPacketID;
	int NextProcessReliablePacketID;
	double TimeSinceLastUpdate;
};
struct PacketIDComparison
{
	bool operator()(const char* lhs, const char* rhs)
	{
		const char* lptr = lhs;
		const char* rptr = rhs;
		lptr += 4;
		rptr += 4;
		int leftPacketID = *((int*)lptr);
		int rightPacketID = *((int*)rptr);
		return leftPacketID > rightPacketID;
	}
};
struct PacketInfo{
	int size;
	char* buffer;
};
union AcknowledgePacket
{
	struct Ack
	{
		char channelID;
		int packetID;
		char packetType; // 1
		char playerID;
	} info;

	char buffer[sizeof(Ack)];
};
union HeartbeatPacket
{
	struct Heartbeat
	{
		char packetType; // 0
		char playerID;
	} info;

	char buffer[sizeof(Heartbeat)];
};

class NetworkingSystem
{
public:
	NetworkingSystem(NetworkType type, const char* ipAddr, u_short port, int playerID = 37, bool nonBlocking = true, bool echoServer = false);
	~NetworkingSystem(void);
	void Update(float deltaSeconds);
	void PushMessage(PacketChannel channel, const char* data, int size);
	void ClearReceiveBuffer();
	void ResetOrderedAndReliablePacketID();

public:
	NetworkType m_type;
	const char* m_ipAddr;
	u_short m_port;
	int m_sendLength;
	std::map<std::string, ClientInfo> m_clinetMap;
	std::vector<char*> m_processList;
	
private:
	void ReceiveMessage();
	void CheckAndReportError(int returnCode);
	void RefreshConnectStatus(float deltaSeconds);
	void WriteIntegerToBuffer(char*& buffer,int data);
	void RefreshSendListFromWaitProcessList();
	void SendAllMessages();
	void RemoveFromResendList(int packetID);
	void HeartBeat(double deltaSeconds);
	int ExtractPacketIDFromPacket(char* packetBuffer);

private:
	bool m_echoServer;
	char* m_tempBuffer;
	int m_addrlen;
	SOCKET m_listen;
	SOCKET m_connect;
	SOCKADDR_IN m_addr;

	std::priority_queue<char*, std::vector<char*>, PacketIDComparison> m_reliableProcessList;
	std::vector<PacketInfo> m_resendList;
	std::vector<PacketInfo> m_sendList;
	std::vector<char*> m_receiveBuffer;
	int m_maxPacketSizeInByte;
	int m_orderedPacketID;
	int m_reliablePacketID;
	int m_nextProcessOrderedPacketID;
	int m_nextProcessReliablePacketID;
	size_t m_maxPacketProcessPerFrame;
	int m_playerID;
	float m_heartbeatCounter;
	FILE* m_file;
};

}

#endif
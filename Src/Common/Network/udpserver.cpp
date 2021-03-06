
#include "stdafx.h"
#include "udpserver.h"
#include <iostream>
#include <process.h> 


using namespace network;

unsigned WINAPI UDPServerThreadFunction(void* arg);


cUDPServer::cUDPServer() :
	m_id(0)
	, m_isConnect(false)
	, m_threadLoop(true)
	, m_bufferLen(0)
	, m_sleepMillis(10)
{
	InitializeCriticalSectionAndSpinCount(&m_CriticalSection, 0x00000400);
}

cUDPServer::~cUDPServer()
{
	m_threadLoop = false;

	::WaitForSingleObject(m_handle, 1000);

	DeleteCriticalSection(&m_CriticalSection);
	closesocket(m_socket);
}


bool cUDPServer::Init(const int id, const int port)
{
	m_id = id;
	m_port = port;

	if (m_isConnect)
	{
		closesocket(m_socket);
		m_isConnect = false;
		m_threadLoop = false;
	}
	else
	{
		std::cout << "Bind UDP Server port = " << port << std::endl;

		if (network::LaunchUDPServer(port, m_socket))
		{
			m_isConnect = true;
			m_threadLoop = true;
			m_handle = (HANDLE)_beginthreadex(NULL, 0, UDPServerThreadFunction, this, 0, (unsigned*)&m_threadId);
		}
		else
		{
			return false;
		}
	}

	return true;
}


void cUDPServer::SetRecvData(const char *buff, const int buffLen)
{
	EnterCriticalSection(&m_CriticalSection);
	memcpy(m_buffer, buff, buffLen);
	m_bufferLen = buffLen;
	m_isReceiveData = true;
	LeaveCriticalSection(&m_CriticalSection);
}


// 받은 패킷을 dst에 저장해서 리턴한다.
// 동기화 처리.
int cUDPServer::GetRecvData(OUT char *dst, const int maxSize)
{
	EnterCriticalSection(&m_CriticalSection);
	int buffLen = 0;
	if (maxSize < m_bufferLen)
	{
		LeaveCriticalSection(&m_CriticalSection);
		return 0;
	}

	if (!m_isReceiveData || (m_bufferLen <= 0))
	{
		m_isReceiveData = false;
		LeaveCriticalSection(&m_CriticalSection);
		return 0;
	}

	memcpy(dst, m_buffer, m_bufferLen);
	buffLen = m_bufferLen;
	m_isReceiveData = false;
	LeaveCriticalSection(&m_CriticalSection);
	return buffLen;
}


void cUDPServer::Close(const bool isWait) // isWait = false
{
	m_threadLoop = false;
	if (isWait)
	{
		::WaitForSingleObject(m_handle, INFINITE);
	}
	m_isConnect = false;
	closesocket(m_socket);
}

// 패킷 전송
int cUDPServer::SendData(const char *buff, const int buffLen)
{
	const int result = send(m_socket, buff, buffLen, 0);
	return result;
}


void PrintBuffer(const char *buffer, const int bufferLen)
{
	for (int i = 0; i < bufferLen; ++i)
		printf("%c", buffer[i]);
	printf("\n");
}


// UDP 서버 쓰레드
unsigned WINAPI UDPServerThreadFunction(void* arg)
{
	cUDPServer *udp = (cUDPServer*)arg;

	while (udp->m_threadLoop)
	{
		const timeval t = { 0, udp->m_sleepMillis }; // ? millisecond
		fd_set readSockets;
		FD_ZERO(&readSockets);
		FD_SET(udp->m_socket, &readSockets);

		const int ret = select(readSockets.fd_count, &readSockets, NULL, NULL, &t);
		if (ret != 0 && ret != SOCKET_ERROR)
		{
			char buff[cUDPServer::BUFLEN];
			const int result = recv(readSockets.fd_array[0], buff, sizeof(buff), 0);
			if (result == SOCKET_ERROR || result == 0) // 받은 패킷사이즈가 0이면 서버와 접속이 끊겼다는 의미다.
			{
				// 에러가 발생하더라도, 수신 대기상태로 계속 둔다.
			}
			else
			{
				udp->SetRecvData(buff, result);
			}
		}

		//Sleep(udp->m_sleepMillis);
	}

	return 0;
}

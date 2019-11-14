#include "stdafx.h"

namespace library
{
	bool IocpServer::Start(Config& config)
	{
		InitConfig = config;
		auto result = CreateListenSocket();
		assert(result == true);

		result = CreateIocp();
		assert(result == true);

		result = CreateMessagePool();
		assert(result == true);

		result = BindListenSocketIocp();
		assert(result == true);

		result = CreateConnections();
		assert(result == true);

		result = CreatePerformance();
		assert(result == true);

		result = CreateWorkerThread();
		assert(result == true);

		Log.Write(LogType::L_INFO, "Server started");
		return true;
	}

	void IocpServer::End()
	{
		if (WorkerIocp != INVALID_HANDLE_VALUE)
		{
			IsWorkerThreadRunnig = false;
			CloseHandle(WorkerIocp);

			for (int i = 0; i < VectorWorkerThread.size(); ++i)
			{
				if (VectorWorkerThread[i].get()->joinable())
				{
					VectorWorkerThread[i].get()->join();
				}
			}
		}

		if (LogicIocp != INVALID_HANDLE_VALUE)
		{
			CloseHandle(WorkerIocp);
		}

		if (ListenSocket != INVALID_SOCKET)
		{
			closesocket(ListenSocket);
			ListenSocket = INVALID_SOCKET;
		}
		WSACleanup();
		DestroyConnections();

		Log.Write(LogType::L_INFO, "Server ended");
	}

	bool IocpServer::ProcessIocpMessage(OUT char& ioType, OUT int& connectionIndex, char* buf, OUT short& copySize, int waitMilliseconds)
	{
		Message* msg = nullptr;
		Connection* connection = nullptr;
		DWORD bytes = 0;
		if (waitMilliseconds == 0)
		{
			waitMilliseconds = INFINITE;
		}

		auto result = GetQueuedCompletionStatus(
			LogicIocp, &bytes,
			reinterpret_cast<PULONG_PTR>(&connection),
			reinterpret_cast<OVERLAPPED**>(&msg),
			waitMilliseconds
		);
		// assert(result == true);
		if (result == false)
		{
			Log.Write(LogType::L_ERROR, "%s | GetQueuedCompletionStatus() failure[%d]", __FUNCTION__, WSAGetLastError());
			return false;
		}

		switch (msg->Type)
		{
			case MessageType::CONNECTION:
			{
				DoPostConnection(connection, msg, ioType, connectionIndex);
				break;
			}
			// 재사용에 딜레이를 주지 않으면 재사용하기 전에 워커 스레드에서 이 세션이 호출될 수도
			case MessageType::CLOSE:
			{
				DoPostClose(connection, msg, ioType, connectionIndex);
				break;
			}
			case MessageType::ONRECV:
			{
				DoPostRecvPacket(connection, msg, ioType, connectionIndex, buf, copySize, bytes);
				UniqueMessagePool.get()->DeallocateMsg(msg);
				break;
			}
		}
		return true;
	}

	void IocpServer::SendPacket(const int connectionIndex, const void* packet, const short packetSize)
	{
		auto connection = GetConnection(connectionIndex);
		if (connection == nullptr)
		{
			Log.Write(LogType::L_ERROR, "%s | GetConnection() failure", __FUNCTION__);
			return;
		}

		char* sendBufReserved = nullptr;
		auto result = connection->ReserveSendPacketBuffer(&sendBufReserved, packetSize);
		if (result == false)
		{
			Log.Write(LogType::L_ERROR, "%s | ReserveSendPacketBuffer() failure", __FUNCTION__);
			return;
		}
	}
}
#pragma once
#include "PacketId.h"

namespace Common
{
#pragma pack(push, 1)
	struct PacketHeader
	{
		unsigned short TotalSize;
		unsigned short Id;
	};

	// 로그인 요청
	const int kMaxUserIdLength = 20;
	const int kMaxUserPwLength = 20;
	const int KPacketHeaderSize = 4;
	struct PacketLoginReq
	{
		char Id[kMaxUserIdLength + 1] = { 0 };
		char Pw[kMaxUserPwLength + 1] = { 0 };
	};
	const size_t kLoginReqPacketSize = sizeof PacketLoginReq;

	struct PacketBasicRes : public PacketHeader
	{
		unsigned short ErrorCode = 0;
	};

	// 입장 알림
	struct PacketNotifyEntrance : public PacketBasicRes
	{
		short UsersCount;
		char UserId[kMaxUserIdLength * 100] = { 0 };
	};

	struct PacketNotifyNewUser : public PacketBasicRes
	{
		char UserId[kMaxUserIdLength] = { 0 };
	};

	const int kMaxRoomChatSize = 256;
	struct PacketRoomChat : public PacketBasicRes
	{
		short IdSize = 0;
		char UserId[kMaxUserIdLength] = { 0 };
		short MsgSize;
		char Msg[kMaxRoomChatSize] = { 0 };
	};

	struct PacketBasicEnterLeaveReq
	{
		short Index;
	};

#pragma pack(pop)
}
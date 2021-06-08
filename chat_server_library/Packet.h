#pragma once
#include "PacketId.h"

namespace Common
{
#pragma pack(push, 1)
	struct PacketHeader
	{
		short total_size_;
		short id_;
	};

	// �α��� ��û
	const int kMaxUserIdLength = 20;
	const int kMaxUserPwLength = 20;
	const int kPacketHeaderLength = 4;
	const int kPacketSizeLength = 2;
	const int kPacketTypeLength = 2;
	struct PacketLoginReq
	{
		char id_[kMaxUserIdLength + 1] = { 0 };
		char pw_[kMaxUserPwLength + 1] = { 0 };
	};
	const size_t kLoginReqPacketSize = sizeof PacketLoginReq;

	struct PacketBasicRes : public PacketHeader
	{
		short error_code_ = 0;
	};

	// ���� �˸�
	struct PacketNotifyEntrance : public PacketBasicRes
	{
		short users_count_;
		char user_id_[kMaxUserIdLength * 100] = { 0 };
	};

	struct PacketNotifyNewUser : public PacketBasicRes
	{
		char user_id_[kMaxUserIdLength] = { 0 };
	};

	const int kMaxRoomChatSize = 256;
	struct PacketRoomChat : public PacketBasicRes
	{
		short id_size_ = 0;
		char user_id_[kMaxUserIdLength] = { 0 };
		short msg_size_;
		char msg_[kMaxRoomChatSize] = { 0 };
	};

	struct PacketBasicEnterLeaveReq : public PacketHeader
	{
		short index_ = -1;
	};

#pragma pack(pop)
}
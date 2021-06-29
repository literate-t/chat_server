#pragma once
#include "PacketId.h"

namespace Common
{
#pragma pack(push, 1)
	struct PacketHeader
	{
		short total_size_;
		PacketId id_;
	};

	// 로그인 요청
	const int kMaxUserIdLength = 30;
	const int kMaxUserPwLength = 30;
	const int kPacketHeaderLength = 4;
	const int kPacketSizeLength = 2;
	const int kPacketTypeLength = 2;
	struct PacketLoginReq
	{
		char id_[kMaxUserIdLength + 1] = { 0 };
		//char pw_[kMaxUserPwLength + 1] = { 0 };
	};
	const size_t kLoginReqPacketSize = sizeof PacketLoginReq;

	struct PacketBasicRes : public PacketHeader
	{
		PacketBasicRes() = default;
		PacketBasicRes(PacketId id, ErrorCode error_code)
		{
			id_ = id;
			error_code_ = error_code;
			total_size_ = sizeof PacketBasicRes;
		}
		ErrorCode error_code_ = ErrorCode::NONE;
	};

	// 입장 알림
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
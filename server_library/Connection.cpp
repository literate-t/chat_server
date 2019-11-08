#include "stdafx.h"

namespace library
{




	int Connection::DecrementRecvIoCount()
	{
		LockGurad lock(cs_);
		return recv_io_cnt_ ? InterlockedDecrement(&recv_io_cnt_) : 0;
	}
	int Connection::DecrementSendIoCount()
	{
		LockGurad lock(cs_);
		return send_io_cnt_ ? InterlockedDecrement(&send_io_cnt_) : 0;
	}
	int Connection::DecrementAcceptIoCount()
	{
		LockGurad lock(cs_);
		return accept_io_cnt_ ? InterlockedDecrement(&accept_io_cnt_) : 0;
	}
}
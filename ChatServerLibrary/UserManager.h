#pragma once
#include "stdafx.h"

namespace Common
{	
	enum class ErrorCode : short;
}

namespace ChatServerLibrary
{
	using ErrorCode = Common::ErrorCode;
	class User;
	class UserManager
	{
	public:
		UserManager();
		~UserManager();

		void Init(const int max_user_count);
		ErrorCode AddUser(const int session, const char* id);
		ErrorCode RemoveUser(const int session_index);
		std::tuple<ErrorCode, User*> GetUser(const int session_index);

		function<void(int, void*, short)> SendPacketFunc;

	private:
		User* AllocateUserFromPoolIndex();
		void ReleaseUserToPoolIndex(const int index);

		User* FindUser(const int session_index);
		User* FindUser(const char* id);

	private:
		std::vector<User>	user_pool_;
		std::queue<int>		user_index_pool_;

		std::unordered_map<int, User*>			user_session_dic_;
		std::unordered_map<const char*, User*>	user_id_dic_;
	};
}
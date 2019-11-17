#pragma once
#include "stdafx.h"
//#include <tuple>
//#include <unordered_map>
//#include <vector>
//#include <queue>

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

		void Init(const int maxUserCount);
		ErrorCode AddUser(const int session, const char* id);
		ErrorCode RemoveUser(const int sessionIndex);
		std::tuple<ErrorCode, User*> GetUser(const int sessionIndex);

		function<void(int, void*, short)> SendPacketFunc;

	private:
		User* AllocateUserFromPoolIndex();
		void ReleaseUserToPoolIndex(const int index);

		User* FindUser(const int sessionIndex);
		User* FindUser(const char* id);

	private:
		std::vector<User>	UserPool;
		std::queue<int>		UserIndexPool;

		std::unordered_map<int, User*>			UserSessionDic;
		std::unordered_map<const char*, User*>	UserIdDic;
	};
}
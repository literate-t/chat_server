#include "stdafx.h"

namespace ChatServerLibrary
{
	UserManager::UserManager()	{}
	UserManager::~UserManager() {}

	void UserManager::Init(const int maxUserCount)
	{
		for (int i = 0; i < maxUserCount; ++i) 
		{
			User user;
			user.Init(static_cast<short>(i));
			UserPool.push_back(user);
			UserIndexPool.push(i);
		}
	}

	User* UserManager::AllocateUserFromPoolIndex()
	{
		if (UserIndexPool.empty()) 
		{
			return nullptr;
		}

		int index = UserIndexPool.front();
		UserIndexPool.pop();

		return &UserPool[index];
	}

	void UserManager::ReleaseUserToPoolIndex(const int index)
	{
		UserIndexPool.push(index);
		UserPool[index].Clear();
	}

	ErrorCode UserManager::AddUser(const int sessionIndex, const char* id)
	{
		if (FindUser(id) != nullptr) 
		{
			return ErrorCode::USER_MGR_ID_DUPLICATION;
		}

		auto user = AllocateUserFromPoolIndex();
		if (user == nullptr) 
		{
			return ErrorCode::USER_MGR_MAX_USER_COUNT;
		}

		user->SetLoginInfo(sessionIndex, id);
		UserSessionDic.insert({ sessionIndex, user});
		UserIdDic.insert({id, user});

		return ErrorCode::NONE;
	}

	ErrorCode UserManager::RemoveUser(const int sessionIndex)
	{
		auto user = FindUser(sessionIndex);
		if (user == nullptr) {
			return ErrorCode::USER_MGR_REMOVE_INVALID_SESSION;
		}

		auto index = user->GetIndex();
		auto id = user->GetId();
		UserSessionDic.erase(sessionIndex);
		UserIdDic.erase(id);
		ReleaseUserToPoolIndex(index);

		return ErrorCode::NONE;
	}

	std::tuple<ErrorCode, User*> UserManager::GetUser(const int sessionIndex)
	{
		auto user = FindUser(sessionIndex);
		if (user == nullptr) 
		{
			return { ErrorCode::USER_MGR_INVALID_SESSION_INDEX, nullptr };
		}

		if (user->IsSet() == false) 
		{
			return { ErrorCode::USER_MGR_NOT_SET_USER, nullptr };
		}

		return { ErrorCode::NONE, user };
	}

	User* UserManager::FindUser(const int sessionIndex)
	{
		auto find_iter = UserSessionDic.find(sessionIndex);
		if (find_iter == UserSessionDic.end()) 
		{
			return nullptr;
		}

		return find_iter->second;
	}

	User* UserManager::FindUser(const char* id)
	{
		auto find_iter = UserIdDic.find(id);
		if (find_iter == UserIdDic.end()) 
		{
			return nullptr;
		}
		return find_iter->second;
	}
}
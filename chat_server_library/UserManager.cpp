#include "stdafx.h"

namespace chat_server_library
{
	UserManager::UserManager()	{}
	UserManager::~UserManager() {}

	void UserManager::Init(const int max_user_count)
	{
		for (int i = 0; i < max_user_count; ++i) 
		{
			User user;
			user.Init(static_cast<short>(i));
			user_pool_.push_back(user);
			user_index_pool_.push(i);
		}
	}

	User* UserManager::AllocateUserFromPoolIndex()
	{
		if (user_index_pool_.empty()) 
		{
			return nullptr;
		}

		int index = user_index_pool_.front();
		user_index_pool_.pop();

		return &user_pool_[index];
	}

	void UserManager::ReleaseUserToPoolIndex(const int user_index)
	{
		user_index_pool_.push(user_index);
		user_pool_[user_index].Clear();
	}

	ErrorCode UserManager::AddUser(const int session_index, const char* id)
	{
		if (nullptr != FindUser(id))
		{
			return ErrorCode::USER_MGR_ID_DUPLICATION;
		}

		auto user = AllocateUserFromPoolIndex();
		if (nullptr == user)
		{
			return ErrorCode::USER_MGR_MAX_USER_COUNT;
		}

		user->SetLoginInfo(session_index, id);
		user_session_dic_.insert({ session_index, user});
		int count = GetCharSize(id);
		char copy_id[Common::kMaxUserIdLength] = {};
		memcpy(copy_id, id, count);
		user_id_dic_.insert({ copy_id, user});

		return ErrorCode::NONE;
	}

	ErrorCode UserManager::RemoveUser(const int session_index)
	{
		auto user = FindUser(session_index);
		if (nullptr == user) {
			return ErrorCode::USER_MGR_REMOVE_INVALID_SESSION;
		}

		auto user_index = user->GetIndex();
		auto user_id = user->GetId();
		user_session_dic_.erase(session_index);
		user_id_dic_.erase(user_id);
		ReleaseUserToPoolIndex(user_index);

		return ErrorCode::NONE;
	}

	std::tuple<ErrorCode, User*> UserManager::GetUser(const int session_index)
	{
		auto user = FindUser(session_index);
		if (nullptr == user)
		{
			return { ErrorCode::USER_MGR_INVALID_SESSION_INDEX, nullptr };
		}

		if (false == user->IsSet())
		{
			return { ErrorCode::USER_MGR_NOT_SET_USER, nullptr };
		}

		return { ErrorCode::NONE, user };
	}

	User* UserManager::FindUser(const int session_index)
	{
		auto find_iter = user_session_dic_.find(session_index);
		if (user_session_dic_.end() == find_iter)
		{
			return nullptr;
		}

		return find_iter->second;
	}

	User* UserManager::FindUser(const char* id)
	{
		auto find_iter = user_id_dic_.find(id);
		if (user_id_dic_.end() == find_iter)
		{
			return nullptr;
		}
		return find_iter->second;
	}

	int UserManager::GetCharSize(const char* str) const
	{
		int count = 0;
		for (int i = 0; i < Common::kMaxUserIdLength; ++i)
		{
			++count;
			if ('\0' == str[i]) return count;
		}
		return 0;
	}
}
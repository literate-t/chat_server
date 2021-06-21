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

	void UserManager::ReleaseUserToPoolIndex(const int index)
	{
		user_index_pool_.push(index);
		user_pool_[index].Clear();
	}

	ErrorCode UserManager::AddUser(const int session_index, const char* id)
	{
		if (nullptr != FindUser(id))
		{
			return ErrorCode::USER_MGR_ID_DUPLICATION;
		}

		auto user = AllocateUserFromPoolIndex();
		if (user == nullptr) 
		{
			return ErrorCode::USER_MGR_MAX_USER_COUNT;
		}

		user->SetLoginInfo(session_index, id);
		user_session_dic_.insert({ session_index, user});
		user_id_dic_.insert({id, user});

		return ErrorCode::NONE;
	}

	ErrorCode UserManager::RemoveUser(const int session_index)
	{
		auto user = FindUser(session_index);
		if (nullptr == user) {
			return ErrorCode::USER_MGR_REMOVE_INVALID_SESSION;
		}

		auto index = user->GetIndex();
		auto id = user->GetId();
		user_session_dic_.erase(session_index);
		user_id_dic_.erase(id);
		ReleaseUserToPoolIndex(index);

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
}
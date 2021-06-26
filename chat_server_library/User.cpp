#include "stdafx.h"

namespace chat_server_library
{
	void User::Init(const short index)
	{
		index_ = index;
	}

	void User::Clear()
	{
		session_index_	= -1;
		id_				= nullptr;
		set_				= false;
		domain_state_ = DomainState::NONE;
		lobby_index_		= -1;
		room_index_		= -1;
	}

	void User::SetLoginInfo(const int session_index, const char* id)
	{
		set_ = true;
		domain_state_ = DomainState::LOGIN;
		session_index_ = session_index;
		id_ = id;
	}

	short User::GetIndex()
	{
		return index_;
	}

	int User::GetSessionIndex()
	{ 
		return session_index_; 
	}

	const char* User::GetId()
	{ 
		return id_; 
	}

	bool User::IsSet()
	{ 
		return set_; 
	}

	short& User::GetLobbyIndex()
	{ 
		return lobby_index_; 
	}

	void User::EnterLobby(const short lobby_index)
	{
		lobby_index_ = lobby_index;
		domain_state_ = DomainState::LOBBY;
	}

	void User::LeaveLobby()
	{
		lobby_index_ = -1;
		//domain_state_ = DomainState::LOGIN;
	}

	short User::GetRoomIndex()
	{ 
		return room_index_;
	}

	void User::EnterRoom(const short lobby_index, const short room_index)
	{
		//lobby_index_ = lobby_index;
		room_index_ = room_index;
		domain_state_ = DomainState::ROOM;
	}

	void User::SetDomainLogin()
	{ 
		domain_state_ = DomainState::LOGIN;
	}

	void User::SetDomainLobby()
	{ 
		domain_state_ = DomainState::LOBBY;
	}

	void User::SetDomainRoom()
	{ 
		domain_state_ = DomainState::ROOM;
	}

	void User::SetDomainClear()
	{
		domain_state_ = DomainState::NONE;
	}

	bool User::IsDomainLogin()
	{ 
		return domain_state_ == DomainState::LOGIN ? true : false;
	}

	bool User::IsDomainLobby()
	{
		return domain_state_ == DomainState::LOBBY ? true : false;
	}

	bool User::IsDomainRoom()
	{ 
		return domain_state_ == DomainState::ROOM ? true : false;
	}
}
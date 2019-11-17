//#include "User.h"
#include "stdafx.h"

namespace ChatServerLibrary
{
	User::User()
	{

	}

	User::~User()
	{

	}

	void User::Init(const short index)
	{
		Index = index;
	}

	void User::Clear()
	{
		SessionIndex = -1;
		Id = nullptr;
		Set = false;
		DomainState = DomainState::NONE;
		LobbyIndex = -1;
		RoomIndex = -1;
	}

	void User::SetLoginInfo(const int sessionIndex, const char* id)
	{
		Set = true;
		DomainState = DomainState::LOGIN;
		SessionIndex = sessionIndex;
		Id = id;
	}

	short& User::GetIndex()
	{
		return Index;
	}

	int& User::GetSessionIndex()
	{ 
		return SessionIndex; 
	}

	const char*& User::GetId()
	{ 
		return Id; 
	}

	bool& User::IsSet()
	{ 
		return Set; 
	}

	short& User::GetLobbyIndex()
	{ 
		return LobbyIndex; 
	}

	void User::EnterLobby(const short lobbyIndex)
	{
		LobbyIndex = lobbyIndex;
		DomainState = DomainState::LOBBY;
	}

	void User::LeaveLobby()
	{
		LobbyIndex = -1;
		DomainState = DomainState::LOGIN;
	}

	void User::LeaveLobbyToEnterRoom()
	{
		DomainState = DomainState::ROOM;
	}

	short& User::GetRoomIndex()
	{ 
		return RoomIndex;
	}

	void User::EnterRoom(const short lobbyIndex, const short roomIndex)
	{
		LobbyIndex = lobbyIndex;
		RoomIndex = roomIndex;
		DomainState = DomainState::ROOM;
	}

	void User::SetDomainLogin()
	{ 
		DomainState = DomainState::LOGIN;
	}

	void User::SetDomainLobby()
	{ 
		DomainState = DomainState::LOBBY;
	}

	void User::SetDomainRoom()
	{ 
		DomainState = DomainState::ROOM;
	}

	bool User::IsDomainLogin()
	{ 
		return DomainState == DomainState::LOGIN ? true : false;
	}

	bool User::IsDomainLobby()
	{
		return DomainState == DomainState::LOBBY ? true : false;
	}

	bool User::IsDomainRoom()
	{ 
		return DomainState == DomainState::ROOM ? true : false;
	}
}
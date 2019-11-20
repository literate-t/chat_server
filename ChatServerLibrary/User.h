#pragma once

namespace ChatServerLibrary
{
	class User
	{
	public:
		enum class DomainState
		{
			NONE = 0,
			LOGIN = 1,
			LOBBY = 2,
			ROOM = 3
		};

	public:
		User()	= default;
		~User() = default;

		void			Init(const short index);		
		void			Clear();
		void			SetLoginInfo(const int sessionIndex, const char* id);
		short&			GetIndex();
		int&			GetSessionIndex();
		const char*&	GetId();
		bool&			IsSet();

		short& GetLobbyIndex();
		void EnterLobby(const short lobbyIndex);
		void LeaveLobby();
		void LeaveLobbyToEnterRoom();

		short& GetRoomIndex();
		void EnterRoom(const short lobbyIndex, const short roomIndex);
		void SetDomainLogin();
		void SetDomainLobby();
		void SetDomainRoom();
		bool IsDomainLogin();
		bool IsDomainLobby();
		bool IsDomainRoom();

	private:
		short Index			= -1;
		int SessionIndex	= -1;
		const char* Id		= nullptr;
		bool Set			= false;
		DomainState DomainState = DomainState::NONE;
		short LobbyIndex	= -1;
		short RoomIndex		= -1;
	};
}
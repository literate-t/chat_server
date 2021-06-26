#pragma once

namespace chat_server_library
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
		void			SetLoginInfo(const int session_index, const char* id);
		short			GetIndex();
		int			GetSessionIndex();
		const char*	GetId();
		bool			IsSet();

		short& GetLobbyIndex();
		void EnterLobby(const short lobby_index);
		void LeaveLobby();

		short GetRoomIndex();
		void EnterRoom(const short lobby_index, const short room_index);
		void SetDomainLogin();
		void SetDomainLobby();
		void SetDomainRoom();
		void SetDomainClear();
		bool IsDomainLogin();
		bool IsDomainLobby();
		bool IsDomainRoom();

	private:
		short index_			= -1;
		int session_index_		= -1;
		const char* id_			= nullptr;
		bool set_				= false;
		DomainState domain_state_ = DomainState::NONE;
		short lobby_index_		= -1;
		short room_index_		= -1;
	};
}
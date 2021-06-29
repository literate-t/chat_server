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
		short			GetIndex() const;
		int				GetSessionIndex() const;
		const char*		GetId() const;
		bool			IsSet() const;

		short GetLobbyIndex() const;
		void EnterLobby(const short lobby_index);
		void LeaveLobby();

		short GetRoomIndex() const;
		void EnterRoom(const short lobby_index, const short room_index);
		void SetDomainLogin();
		void SetDomainLobby();
		void SetDomainRoom();
		void SetDomainClear();
		bool IsDomainLogin() const;
		bool IsDomainLobby() const;
		bool IsDomainRoom() const;

	private:
		int GetCharSize(const char* str) const;

	private:
		short index_			= -1;
		int session_index_		= -1;
		char id_[Common::kMaxUserIdLength]		= {};
		bool set_				= false;
		DomainState domain_state_ = DomainState::NONE;
		short lobby_index_		= -1;
		short room_index_		= -1;
	};
}
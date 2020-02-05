#pragma once
namespace tbsg {

	/**
	 * \brief Lobby-related commands that the server can send to the client.
	 * LobbyServer -> Client
	 */
	enum class ServerLobbyCommands
	{
		/**
		 * \brief server send list of lobbies
		 * \param unsigned int lobby numbers
		 * \param unsinged int lobbyId
		 * \param unsinged int ownerConnectionId (0 == no owner)
		 * \param unsinged int opponentConnectionId (0 == no opponent)
		 * \param bool inGame
		 */
		LobbiesListed,

		/**
		 * \brief Server sends to client when lobby was created
		 * \param unsinged int lobbyId
		 */
		LobbyCreated,

		/**
		 * \brief Server sends to client when they are already in a lobby.
		 */
		LobbyCreationFailed,

		/**
		 * \brief server sends to opponent client that the host left
		 * \param unsinged int LobbyID
		 */
		LobbyDestroyed,

		/**
		 * \brief server sends to the client who is leaving
		 * \note gets called: When opponent leaves, when owner leaves
		 * \param unsinged int profileId (the client which left)
		 */
		LobbyLeft,

		/**
		 * \brief The server will send this after ClientLobbyCommands::LeaveLobby if the player isn't in a lobby.
		 */
		LobbyLeftFailed,

		/**
		 * \brief server sends to lobby owner client
		 * \param unsinged int playerId (the player which left)
		 */
		LobbyPlayerLeft,

		/**
		 * \brief Server sends this packet when the lobby owner has been changed.
		 * \param unsinged int connectionId, The new owner of the lobby.
		 */
		LobbyOwnershipTransferred,

		/**
		 * \brief server sends in case th lobby does not exist or the lobby is full
		 * \param LobbyError lobbyError
		 */
		JoinLobbyFailed,

		/**
		 * \brief server sends to client (joiner owner) that the join was successful
		 * \param unsinged int connectionId (owner)
		 * \param unsinged int lobbyId
		 */
		JoinLobbySuccess,

		/**
		 * \brief server sends to client (lobby owner) if player joins
		 * \param unsinged int clientId
		 */
		LobbyPlayerJoined,

		SelectedDeck,
		OpponentSelectedDeck,
		SelectingDeckFailed,

		/**
		 * \brief server sends to client(s) that the game started
		 * \param std::string ip The ip of the GameServer
		 * \param unsigned int port The port of the GameServer
		 */
		LobbyGameStarted,

		/**
		 * \brief server sends to client(s) that the starting of the game failed
		 * \brief (Eg. No GameServers available)
		 */
		LobbyGameStartFailed,

		/**
		 * \brief server notifies clients that game stopped
		 * \param unsigned int lobbyId
		 */
		LobbyGameStopped,

		/**
		 * \brief server gives the client the name of the profile
		 * \param unsigned int profileId
		 * \param std::string the name of the player
		 */
		ProfileName
	};

	inline std::string GetName(ServerLobbyCommands command)
	{
		switch (command)
		{
			case ServerLobbyCommands::LobbiesListed: return "LobbiesListed";
			case ServerLobbyCommands::LobbyCreated: return "LobbyCreated";
			case ServerLobbyCommands::LobbyDestroyed: return "LobbyDestroyed";
			case ServerLobbyCommands::LobbyLeft: return "LobbyLeft";
			case ServerLobbyCommands::LobbyLeftFailed: return "LobbyLeftFailed";
			case ServerLobbyCommands::LobbyPlayerLeft: return "LobbyPlayerLeft";
			case ServerLobbyCommands::LobbyOwnershipTransferred: return "LobbyOwnershipTransferred";
			case ServerLobbyCommands::JoinLobbyFailed: return "JoinLobbyFailed";
			case ServerLobbyCommands::JoinLobbySuccess: return "JoinLobbySuccess";
			case ServerLobbyCommands::LobbyPlayerJoined: return "LobbyPlayerJoined";
			case ServerLobbyCommands::LobbyGameStarted: return "LobbyGameStarted";
			case ServerLobbyCommands::LobbyGameStopped: return "LobbyGameStopped";
			case ServerLobbyCommands::LobbyGameStartFailed: return "LobbyGameStartFailed";
			case ServerLobbyCommands::SelectedDeck: return "SelectedDeck";
			case ServerLobbyCommands::OpponentSelectedDeck: return "OpponentSelectedDeck";
			case ServerLobbyCommands::SelectingDeckFailed: return "SelectingDeckFailed";
			case ServerLobbyCommands::ProfileName: return "ProfileName";
			case ServerLobbyCommands::LobbyCreationFailed: return "LobbyCreationFailed";
			default: ;
		}
		assert(false && "Enum type not implemented! GetName(ServerLobbyCommands)");
		return "Enum type not implemented! GetName(ServerLobbyCommands)";
	}

	/**
	 * \brief Lobby-related commands that the client can send to the server.
	 * Client -> LobbyServer
	 */
	enum class ClientLobbyCommands
	{
		/**
		 * \brief client requests lobbies
		 */
		ListLobbies,

		/**
		 * \brief client send when it is intended to create a lobby
		 */
		CreateLobby,

		/**
		 * \brief client sends to server in order to notify the server that the client wishes to join
		 * \param unsinged int lobbyId
		 */
		JoinLobby,

		/**
		 * \brief client send when it is intended to leave a lobby.
		 */
		LeaveLobby,

		/**
		 * \brief client send when it selected a deck. The server will remember this when starting the game.
		 */
		SelectDeck,

		/**
		 * \brief client send to server to say the game starts
		 * \note the server will respond with LobbyGameStarted
		 */
		LobbyGameStart,

		/**
		 * \brief client requests game to stop
		 * \warning Unused
		 */
		LobbyGameStop,

		/**
		 * \brief client send to server to start a singleplayer AI game.
		 * \note the server will respond with LobbyGameStarted
		 */
		AIGameStart,

		/**
		 * \brief client request the name of a profile
		 * \param unsigned int profileId to request the name of
		 */
		GetProfileName
	};

	inline std::string GetName(ClientLobbyCommands command)
	{
		switch (command)
		{
			case ClientLobbyCommands::ListLobbies: return "ListLobbies";
			case ClientLobbyCommands::CreateLobby: return "CreateLobby";
			case ClientLobbyCommands::JoinLobby: return "JoinLobby";
			case ClientLobbyCommands::LeaveLobby: return "LeaveLobby";
			case ClientLobbyCommands::SelectDeck: return "SelectDeck";
			case ClientLobbyCommands::LobbyGameStart: return "LobbyGameStart";
			case ClientLobbyCommands::LobbyGameStop: return "LobbyGameStop";
			case ClientLobbyCommands::GetProfileName: return "GetProfileName";
			case ClientLobbyCommands::AIGameStart: return "AIGameStart";
		}
		assert(false && "Enum type not implemented! GetName(ClientLobbyCommands)");
		return "Enum type not implemented! GetName(ClientLobbyCommands)";
	}

	/**
	 * \brief All errors with the lobby
	 */
	enum class LobbyError
	{
		Unknown,
		Full,
		Exist,
		Started,
		AlreadyJoined,
		OnlyTheOwnerCanStartTheGame,
		NoGameServerAvailable,
		NoDeckSelected
	};

}
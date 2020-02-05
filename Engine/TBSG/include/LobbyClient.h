#pragma once
#include "Net/Client.h"
#include "utils/EventQueues.h"
#include "memory/smart_ptr.h"

class ClientApplication;

class LobbyClient : public net::Client
{
public:
	//default constructor
	LobbyClient() = default;

	void Initialize(ptl::shared_ptr<LobbyEventQueue> lobbyEventQueue, ClientApplication* clientApplication);
	void Update();

	// *** Override Functions *** //
	void OnConnect() override;
	void OnDisconnect() override;
	void OnIdentificationSuccess() override;
	void OnIdentificationFailure(net::IdentifyResponse response) override;

	void RequestLobbyList();
	void RequestProfileName(unsigned int profileId) const;

	/**
	 * \brief Retrieve the token associated with the username and password that was used during login
	 * \see WebRequestPC.h / WebRequestPS4.h
	 */
	const ptl::string& GetNetworkAccessToken() const { return accessToken; }
	void SetNetworkAccessToken(const ptl::string& newToken) { accessToken = newToken; }
	void SetCurrentUserName(const ptl::string& newUsername) { userName = newUsername; }
private:
	void GetIdentity(Packet& packet) override;
	void HandleCustomPacket(unsigned customCommand, Packet& packet) override;

	bool joinedLobby{ false };

	ClientApplication* clientApplication{ nullptr };

	// Network Token
	ptl::string accessToken{};
	ptl::string userName{};
	ptl::shared_ptr<LobbyEventQueue> lobbyEventQueue;

	ptl::string prefix = "LobbyClient";
};

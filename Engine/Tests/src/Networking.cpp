#ifdef PLATFORM_WINDOWS
#include <catch/catch.hpp>

#include "core/StrHash.h"
#include "Net/Client.h"
#include "Net/Server.h"
#include "Net/ClientInterface.h"
#include "Net/ServerInterface.h"

#include <functional>

struct MockPacketResult {
	int command;
	Packet packet;
};

class MockClient : public ClientInterface {

	void OnLoginSuccessful(unsigned int playerId) {
		this->playerId = playerId;
	}

	void OnApplicationCommandReceived(int command, Packet&& packet) override {
		packets.push({ command, Packet(packet) });
	}

	void OnLobbyCommandReceived(int command, Packet&& packet) override {
		packets.push({ command, Packet(packet) });
	}
public:
	std::queue<MockPacketResult> packets;
	int playerId = -1;
};

class MockServer : public ServerInterface {
	void OnApplicationCommandReceived(int playerId, int command, Packet&& packet) {
		packets.push({ command, Packet(packet) });
	}

	void OnLobbyCommandReceived(int playerId, int lobbyId, int command, Packet&& packet) {
		packets.push({ command, Packet(packet) });
	}

public:
	std::queue<MockPacketResult> packets;
};

TEST_CASE("[Networking] connecting, sending and stress testing")
{
	Server* server = new Server();
	MockServer serverInterface;
	server->AddObserver(&serverInterface);
	server->StartServer(1111, 1);

	Client* client = new Client();
	MockClient clientInterface;
	client->AddObserver(&clientInterface);

	std::function<void(std::function<bool()>)> waitOrTimeOut = ([client, server](std::function<bool()> wait) {
		unsigned int timeoutCounter = 0;
		while (timeoutCounter < 5000 && wait()) {
			client->Update();
			server->Update();
			timeoutCounter++;

			Sleep(1);
		}
	});

	{ // Connecting and retrieving a player ID
		client->Connect("127.0.0.1", 1111);
		waitOrTimeOut([client, &clientInterface]() {
			return !client->IsConnected() || clientInterface.playerId == -1;
		});

		REQUIRE(client->IsConnected());
		REQUIRE(clientInterface.playerId != -1);
	}

	{ // Sending a package with a lot of text
		Packet mockPacket1;
		std::string mockPacket1Text = "Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium doloremque laudantium, totam rem aperiam, eaque ipsa quae ab illo inventore veritatis et quasi architecto beatae vitae dicta sunt explicabo. Nemo enim ipsam voluptatem quia voluptas sit aspernatur aut odit aut fugit, sed quia consequuntur magni dolores eos qui ratione voluptatem sequi nesciunt. Neque porro quisquam est, qui dolorem ipsum quia dolor sit amet, consectetur, adipisci velit, sed quia non numquam eius modi tempora incidunt ut labore et dolore magnam aliquam quaerat voluptatem. Ut enim ad minima veniam, quis nostrum exercitationem ullam corporis suscipit laboriosam, nisi ut aliquid ex ea commodi consequatur? Quis autem vel eum iure reprehenderit qui in ea voluptate velit esse quam nihil molestiae consequatur, vel illum qui dolorem eum fugiat quo voluptas nulla pariatur?";
		mockPacket1 << mockPacket1Text;

		client->SendGamePacket(1, std::move(mockPacket1));
		printf("SENDING LARGE PACKET\n");

		waitOrTimeOut([&serverInterface]() {
			return serverInterface.packets.size() == 0;
		});

		REQUIRE(serverInterface.packets.size() > 0);

		MockPacketResult mockPacketResult1 = serverInterface.packets.front();
		serverInterface.packets.pop();

		std::string mockPacket1TextResult;
		mockPacketResult1.packet >> mockPacket1TextResult;

		printf("RECEIVED LARGE PACKET\n");

		REQUIRE(mockPacketResult1.command == 1);
		REQUIRE(mockPacket1TextResult.compare(mockPacket1Text) == 0);

		printf("VERIFIED LARGE PACKET\n");
	}

	{ // Sending a lot of packets / stress testing
		std::queue<std::string> dataToSend;

		for (unsigned int i = 0; i < 500; i++) {
			std::string data = std::to_string(tbsg::SimpleHash(("PACKET " + std::to_string(i)).c_str()));
			dataToSend.push(data);

			Packet packet;
			packet << data;

			client->SendGamePacket(2, std::move(packet));
		}

		printf("SENDING %i PACKETS\n", static_cast<int>(dataToSend.size()));

		waitOrTimeOut([&dataToSend, &serverInterface]() {
			return serverInterface.packets.size() < dataToSend.size();
		});

		printf("RECEIVED %i PACKETS\n", static_cast<int>(serverInterface.packets.size()));

		REQUIRE(serverInterface.packets.size() == dataToSend.size());

		while (dataToSend.size() > 0) {
			MockPacketResult mockPacketResult = serverInterface.packets.front();
			std::string expectedResult = dataToSend.front();
			dataToSend.pop();
			serverInterface.packets.pop();

			std::string mockPacketTextResult;
			mockPacketResult.packet >> mockPacketTextResult;

			REQUIRE(mockPacketResult.command == 2);
			REQUIRE(mockPacketTextResult.compare(expectedResult) == 0);
		}

		printf("VERIFIED PACKETS\n");
	}

	{ // Bounce packets between server and client
		std::queue<std::string> dataToSend;

		for (unsigned int i = 0; i < 10; i++) {
			std::string data = std::to_string(tbsg::SimpleHash(("PACKET " + std::to_string(i)).c_str()));
			dataToSend.push(data);

			Packet packet;
			packet << data;

			client->SendGamePacket(3, std::move(packet));
		}

		printf("SENDING %i PACKETS TO SERVER\n", static_cast<int>(dataToSend.size()));

		waitOrTimeOut([&dataToSend, &serverInterface]() {
			return serverInterface.packets.size() < dataToSend.size();
		});

		REQUIRE(serverInterface.packets.size() == dataToSend.size());

		printf("RETURNING %i PACKETS TO CLIENT\n", static_cast<int>(serverInterface.packets.size()));

		while (serverInterface.packets.size() > 0) {
			MockPacketResult mockPacketResult = serverInterface.packets.front();
			serverInterface.packets.pop();

			std::string mockPacketTextResult;
			mockPacketResult.packet >> mockPacketTextResult;

			Packet returnPacket;
			returnPacket << mockPacketTextResult;

			server->SendApplicationPacket(clientInterface.playerId, 3, std::move(returnPacket));
		}

		waitOrTimeOut([&dataToSend, &clientInterface]() {
			return clientInterface.packets.size() < dataToSend.size();
		});

		REQUIRE(clientInterface.packets.size() == dataToSend.size());

		printf("RECEIVED %i PACKETS AT CLIENT\n", static_cast<int>(clientInterface.packets.size()));

		while (dataToSend.size() > 0) {
			MockPacketResult mockPacketResult = clientInterface.packets.front();
			std::string expectedResult = dataToSend.front();
			dataToSend.pop();
			clientInterface.packets.pop();

			std::string mockPacketTextResult;
			mockPacketResult.packet >> mockPacketTextResult;

			REQUIRE(mockPacketResult.command == 3);
			REQUIRE(mockPacketTextResult.compare(expectedResult) == 0);
		}

		printf("VERIFIED PACKETS AT CLIENT\n");
	}


	{ // Disconnecting
		client->Disconnect();

		waitOrTimeOut([client]() {
			return client->IsConnected();
		});

		REQUIRE(!client->IsConnected());
	}


	delete client;
	delete server;
}
#endif
#pragma once

/**
 * \brief Generic Server commands.
 * \warning make use of Client::SendPacket
 * \see Client::SendPacket
 * \ingroup Network 
 */
enum class NetCommands
{
	/**
	 * \brief Sent by client. First command the client should sent to the Lobby or Gameserver to identify itself.
	 * \param std::string The session token of the client that it got provided from the Login WebApp
	 */
	Identify,
	/**
	 * \brief Sent by server after NetCommands::Identify. Sent when the identification was valid.
	 */
	IdentifySuccessful,
	/**
	 * \brief Sent by server after NetCommands::Identify. Sent when the identification was invalid.
	 */
	IdentifyFailure,
	/**
	 * \brief Sent by server if the client is trying to execute a custom command while not being identified.
	 */
	NotIdentified,

	/**
	 * \brief Sent by server if the client has connected but the server has encryption disabled.
	 */
	ConnectedWithoutEncryption,

	HandshakeServerKey,
	HandshakeDataKey,
	HandshakeSuccess,
	HandshakeFailed,

	CryptoPacket,
    /**
	 * \brief Custom command that will be passed to the custom implementation.
	 */
	CustomCommand
};

inline std::string GetName(NetCommands command)
{
	switch(command)
	{
		case NetCommands::Identify: return "Identify";
		case NetCommands::IdentifySuccessful: return "IdentifySuccessful";
		case NetCommands::IdentifyFailure: return "IdentifyFailure";
		case NetCommands::NotIdentified: return "NotIdentified";
		case NetCommands::ConnectedWithoutEncryption: return "ConnectedWithoutEncryption";
		case NetCommands::HandshakeServerKey: return "HandshakeServerKey";
		case NetCommands::HandshakeDataKey: return "HandshakeDataKey";
		case NetCommands::HandshakeSuccess: return "HandshakeSuccess";
		case NetCommands::HandshakeFailed: return "HandshakeFailed";
		case NetCommands::CryptoPacket: return "CryptoPacket";
		case NetCommands::CustomCommand: return "CustomCommand";
	}
	return "Unknown";
}

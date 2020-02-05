#pragma once
/**
* \Struct to send over data from the Lobby object to UI
*/
struct LobbyEntity
{
    unsigned int LobbyID{};
    unsigned int ownerID{};
    unsigned int playerCount{};
    bool inGame = false;
};
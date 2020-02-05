#pragma once
#include <Payloads.h>
#include "memory/Containers.h"
#include "core/SparseSet.h"
#include <climits>
#include "DataStructs.h"
#include "memory/smart_ptr.h"

class Packet;
/**
 * \brief Essential the database for our game. It contains, manages all the data. It gets filled by the Controller after the first connection. To ensure the game data is valid
 */
class GameDatabase
{
public:
    GameDatabase() = default;
    GameDatabase(const GameDatabase& other)= default;

    /**
     * \brief 
     * \param netID 
     */
    void SetNetworkId(unsigned int netID);

	void InitMaps(std::size_t size);

	void AddMaps(Packet&& packet);
	void AddMaps(unsigned int id, ptl::vector<ptl::string>&& maps);
	tbsg::Card* GetCard(unsigned int id);
	const ptl::vector<ptl::string>& GetMap(unsigned int id);

	// Add values based on JSON loading
	void AddCards(ptl::vector<tbsg::Card>&& passedCards) { cards = std::move(passedCards); };
	void AddCardRarities(ptl::vector<tbsg::CardRarity>&& passedcardRarities) { cardRarities = std::move(passedcardRarities); };
	void AddCardTypes(ptl::vector<tbsg::CardType>&& passedcardTypes) { cardTypes = std::move(passedcardTypes); };
	void AddMonsterCards(ptl::vector<tbsg::MonsterCard>&& passedmonsterCards) { monsterCards = std::move(passedmonsterCards); };
	void AddMonsterDecks(ptl::vector<tbsg::MonsterDeck>&& passedmonsterDecks) { monsterDecks = std::move(passedmonsterDecks); };
	void AddDecks(ptl::vector<tbsg::Deck>&& passedPlayerDecks) { playerDecks = std::move(passedPlayerDecks); }
    
    //Debug of sending card
    unsigned int GetLastPlacedCard() { return m_LastPlacedCard; }
    void SetLobbySelected(unsigned int id) { m_LobbySelected = id; }
    unsigned int GetLobbySelected() { return m_LobbySelected; }

	ptl::vector<tbsg::Card>& GetCardList() { return cards; }
	const ptl::vector<tbsg::Deck>& GetDeckList() { return playerDecks; }
private:
    //Network data
    unsigned int m_NetworkID{ UINT_MAX };
    unsigned int m_LastPlacedCard = 1234567;  // Used for sending the last placed card ID (Only for debug)
    unsigned int m_LobbySelected = 0;

    tbsg::Profile m_profile{};
    //keeps track of all the m_Units the system knows based on the current player (profile)
    ptl::vector<tbsg::Unit*> m_Units{};

	ptl::vector<tbsg::Card> cards{};
	ptl::vector<tbsg::CardRarity> cardRarities{};
	ptl::vector<tbsg::CardType> cardTypes{};
	ptl::vector<tbsg::MonsterCard> monsterCards{};
	ptl::vector<tbsg::MonsterDeck> monsterDecks{};
	ptl::vector<tbsg::Deck> playerDecks{};

	//TO BE REMOVED...
	ptl::sparse_set<unsigned int, ptl::vector<ptl::string>> m_maps{};
	ptl::sparse_set<unsigned int, tbsg::Card> m_cards{};

    // GamePlay data:
    unsigned int m_currentChapter{};
    tbsg::Round m_currentRound{};
    tbsg::ResultOfRound m_CurrentResultOfRound{};
};

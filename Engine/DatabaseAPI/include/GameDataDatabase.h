#pragma once
#include "Payloads.h"
#include "core/SparseSet.h"
#include "memory/String.h"

#include <memory>
#include <mariadb++/connection.hpp>
#include "DatabaseAPI.h"
class Packet;

namespace tbsg {
	enum class CardDataType : unsigned int
	{
		None = 0, BaseCost = 1, Family = 2, Type = 3, Health = 4, Attack = 5,
	};

	namespace  scripting
	{
		class LuaContext;
	}

	/**
	 * \brief The database of the Server.
	 * \ingroup Gameplay
	 */
	class GameDataDatabase
	{
	public:
		GameDataDatabase() = default;

		/**
		 * \brief Loads the data from the database. It also binds the datamodel to lua.
		 */
		void Initialize(db::DatabaseAPI* api);
		/**
		 * \brief Loads the decks for the specified profile from the database.
		 */
		void LoadDecksOfProfile(unsigned int profileId);
		/**
		 * \brief Loads the specified deck from the database.
		 * \warning Should only be called once! Use GetMonsterDeck(deckId) to get the actual object after loading.
		 * \return bool Whether loading succeeded.
		 */
		bool LoadMonsterDeck(unsigned int deckId);
		void LoadMonsterDecks();
		/**
		 * \brief Loads the specified deck from the database.
		 * \warning Should only be called once! Use GetMonsterDeck(deckName) to get the actual object after loading.
		 * \return bool Whether loading succeeded.
		 */
		bool LoadMonsterDeck(const ptl::string& deckName);

		const ptl::sparse_set<unsigned int, Card>& GetCards() const;
		Card* GetCard(unsigned int id);
        Card* GetCard(std::string name);

		const ptl::sparse_set<unsigned int, MonsterCard>& GetMonsterCards() const;
		MonsterCard* GetMonsterCard(unsigned int id);

		const ptl::sparse_set<unsigned, tbsg::CardType>& GetCardTypes() const;
		const ptl::sparse_set<unsigned, tbsg::CardRarity>& GetCardRarity() const;

		const Script* GetScript(unsigned int id);
		const Script* GetScriptForCard(unsigned int cardId);
		const Script* GetScriptForMonsterCard(unsigned int monsterCardId);
		/**
		 * \brief Get Script by name. (Only works in scripts that have a name (only 'other' scripts))
		 */
		const Script* GetScript(const ptl::string& name);

		const Reward* GetReward(unsigned id);
		const ptl::sparse_set<unsigned, tbsg::Reward>& GetRewards() const;

		/**
		 * \brief Get the decks of playerId.
		 * \param profileId The profile to get the decks of.
		 */
		ptl::vector<Deck>& GetDecks(unsigned int profileId);
		Deck* GetDeck(unsigned int profileId, unsigned int id);
		Deck* GetDeck(unsigned int profileId, const ptl::string& name);

		const ptl::sparse_set<unsigned int, tbsg::MonsterDeck>& GetMonsterDecks();
		tbsg::MonsterDeck* GetMonsterDeck(unsigned int deckId);
		const tbsg::MonsterDeck* GetMonsterDeck(const ptl::string& deckName);
        ptl::vector<unsigned int> GetCardsByType(const ptl::string type);

		/**
		 * \brief Creates a file on the harddrive of the requested script where the executable is located
		 * \return bool Whether creating succeeded.
		 */
		bool CreateHarddriveCopyOfScript(const ptl::string& requestedString);
	private:
		db::DatabaseAPI* api{};

		ptl::sparse_set<unsigned int, Card> cards{};
		ptl::sparse_set<unsigned int, MonsterCard> monsterCards{};
		ptl::sparse_set<unsigned int, CardType> cardTypes{};
		ptl::sparse_set<unsigned int, CardRarity> cardRarity{};
		ptl::sparse_set<unsigned int, Script> scripts{};
		ptl::sparse_set<unsigned int, Reward> rewards{};
		ptl::unordered_map<unsigned int, ptl::vector<Deck>> decks{};
		ptl::sparse_set<unsigned int, MonsterDeck> monsterDecks{};
	};
}

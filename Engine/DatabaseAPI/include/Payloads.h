#pragma once

#include "memory/string.h"
#include "memory/Containers.h"

namespace tbsg
{

	/*! \addtogroup Gameplay
	 *  @{
	 */

	 /**
	  * \brief forward ref
	  * \see Unit
	  */
	struct Unit;

	/**
	 * \brief
	 */
	enum class CardRewardType
	{
		None = 0,
		Weapon,
		Attack,
		Resource,
		Armor,
		Health
	};

	/**
	 * \brief All the different types of stats that are used in the game.
	 */
	enum class StatType
	{
		None = 0,
		Health,
		Attack,
		Armor,
		Resource,
		WeaponAttack,
		WeaponDurability
	};

	enum class EffectChange
	{
		None = 0,

		Card_Health,
		Card_Attack,
		Card_Armor,
		Card_Particle,
		Card_Animation,
        Card_Death,

		Hero_Weapon_Attack,
		Hero_Weapon_Durability,
		Hero_Health,
		Hero_Resource,
		Hero_Armor,
		Hero_Attack,
		Hero_Particle,
		Hero_Animation,
		Hero_Death,
        Hero_WinLose,

		Reward_Attack,
		Reward_Health,
		Reward_Armor,
		Reward_Resource,
		Reward_Weapon_Attack,
		Reward_Weapon_Durability,

		Deck_Shuffle,
		Deck_DiscardToDeck,
		Deck_AddCard,
		Deck_RemoveCard,
		Deck_DrawCard,

		Hand_AddCard,
		Hand_RemoveCard,

		Discard_AddCard,
		Discard_RemoveCard,

		NextMonster,
		NextStartingPlayer,
		Play_Sound
	};

	enum class BaseEffect
	{
		None = 0,
		MonsterDamage,
		OpponentDamage,
		SelfDamage,

		MonsterHealth,
		OpponentHealth,
		SelfHealth,

		MonsterArmor,
		OpponentArmor,
		SelfArmor,

		OpponentDrawCard,
		SelfDrawCard,

		OpponentDiscardCard,
		SelfDiscardCard
	};


	/**
	 * \brief MetaData contains the name and description of a card
	 */
	struct MetaData
	{
		ptl::string name{};
		ptl::string description{};
        unsigned int rarity{};
        unsigned int type{};
	};


	/**
	 * \brief Weapon, describes the stats of the weapon
	 */
	struct Weapon
	{
		unsigned int id{};
		unsigned int attack{};
		unsigned int durability{};
	};


	/**
	 * \brief Used after fighting a card that is a monster, will be be given to a Hero.
	 */
	struct Reward
	{
		CardRewardType type{};
		unsigned int powerup{};
		Weapon* weapon = nullptr;
	};


	struct BaseCardEffects
	{
		BaseEffect baseEffect{};
		int effectValue{};
	};


	/**
	 * \brief All stats related to the behaviour of the card
	 */
	struct CardData
	{
		ptl::vector<BaseCardEffects> baseCardEffects{};
	};

    struct MonsterData
    {
        unsigned int health{};
        unsigned int maxHealth{};
        unsigned int armor{ 0 };
        unsigned int monsterTrait{};
        ptl::vector<Reward*> reward{};
    };

	/**
	 * \brief The data representation of a player in a match.
	 */
	struct Hero
	{
		unsigned int health{ 40 };
		unsigned int maxHealth{ 40 };
		unsigned int resource{ 0 };
		unsigned int armor{ 0 };
		unsigned int attack{ 1 };
		unsigned int baseAttack{ 1 };
		Weapon* weapon{ nullptr };
	};

	/**
	 * \brief Representation of any Lua script that be used in related functions
	 */
	struct Script
	{
		unsigned int id{};
		/// != 0 when the script is for a card.
		unsigned int cardId{ 0 };
		/// != 0 when the script is for a monster card.
		unsigned int monsterCardId{ 0 };
		/// != "" when the script is not for any (monster) card.
		ptl::string name{};
		ptl::string code{};
	};

	/**
	 * \brief The card object that contains all possible data that makes the card.
	 */
	struct Card
	{
		unsigned int id{};
		MetaData meta{};
		CardData data{};
	};
    
    struct MonsterCard
    {
        unsigned int id{};
        MetaData meta{};
        MonsterData data{};
    };

	struct CardRarity
	{
		unsigned int id{};
		ptl::string name{};
	};

	struct CardType
	{
		unsigned int id{};
		ptl::string name{};
	};

	/**
	 * \brief A deck of cards
	 */
	struct Deck
	{
		unsigned int id{};
		ptl::string name{};
		ptl::vector<Card*> cards{};
	};

    /**
    * \brief A deck of cards
    */
    struct MonsterDeck
    {
        unsigned int id{};
        ptl::string name{};
        ptl::vector<MonsterCard*> cards{};
    };

	/**
	 * \brief Representation of a player in the match with its personal information
	 */
	struct Profile
	{
		unsigned int id{};
		ptl::string username{};
		unsigned int connectionId{ 0 }; // 0 means that there is no connection with this profile
	};

	/**
	 * \brief Submitted struct that indicate who submitted the played cards.
	 */
	struct Play
	{
		// player index in match vector of players:
		unsigned int playerIndex{};
		// card index of cards in vector of cards in hand;
		unsigned int playedCard{};
	};

	/**
	* \brief Change contains data about what changed after a card is played, based on the type it becomes clear what the information is about.
			 Containing both states and the index that is affected.
	*/
	struct Change
	{
		/** containing the type of what changed */
		EffectChange changeType{};
		/** expresses the change */
		int change{};
		/** index of affected object. e.g Hero index 0 or 1, playedCard index 0 til 3*/
		unsigned int index{};
	};

	/**
	 * \brief ResultsOfRound contains all the data of one played around between players, data such as the played card ID's and what changes were with that.
	 */
	struct ResultOfRound
	{
		ptl::vector<unsigned int> playedCards{ 2 };
		ptl::vector<Change> results{};
	};

	/*! @} */

	struct ClientMatchState
	{
        //Monsters
        ptl::vector<tbsg::MonsterCard> monsterCards{ };

        //Heroes
        ptl::vector<tbsg::Hero> heroes{ };

        ptl::vector<ptl::vector<unsigned int>> playerDecks{ 2 };
        ptl::vector<ptl::vector<unsigned int>> playerHands{ 2 };
        ptl::vector<ptl::vector<unsigned int>> playerDiscards{ 2 };
	};

	/**
	 * \brief Struct for creating and handling a match from the database.
	 */
	struct Match
	{
		unsigned int id{}; // Filled by the DatabaseAPI.
		unsigned int ownerProfileId{};
		unsigned int ownerDeckId{};
		unsigned int opponentProfileId{}; // 0 for AI game
		unsigned int opponentDeckId{}; // 0 for AI game
		unsigned int serverId{};
	};

	struct Server
	{
		unsigned int id{};
		std::string hostname{};
		std::string ip{};
		unsigned int port{};
		bool occupied{};
	};

	struct LobbyData
	{
		unsigned int id{};
		unsigned int ownerConnectionId{};
		ptl::string ownerName{};
		unsigned int opponentConnectionId{};
		ptl::string opponentName{};
		bool inGame{};
	};

    struct EffectEvent
    {
        ptl::string customEventName;
        /**
         * \brief Is the representation of an API or LuaFunction call
         */
        std::function<bool()> eventDelegate;
    };
}

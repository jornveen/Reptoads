#pragma once
#include "memory/Containers.h"
#include "Payloads.h"
#include "gameplay/DataStructs.h"
#include <future>
#include <rapidjson/document.h>

namespace tbsg
{
    struct Hero;
    class GameDataDatabase;
    /*! \addtogroup Gameplay
     *  Our AI System
     *  @{
     */

    struct PlayerChanges
    {
        unsigned int healingDone    = 0;
        unsigned int damageTaken    = 0;
        unsigned int armorGained    = 0;
        unsigned int armorLost      = 0;
    };

    struct PlayedCard
    {
        Card selectedCard;
        MatchData matchDataBefore;
        MatchData matchDataAfter;
        ptl::vector<PlayerChanges> playerChanges{ 2 };
        bool isCardPlayed   = false;
        float utilityScore  = 0;
        //Card followUpCard
    };

    struct TurnInfo
    {
        MatchData matchDataAfter;
        std::pair<ptl::vector<PlayedCard>, ptl::vector<PlayedCard>> playedCards;
        ptl::vector<PlayerChanges> playerChanges{ 2 };
    };

    struct GameInfo
    {
        ptl::vector<TurnInfo> turnInfo;
    };

    class AISystem
    {
    public:
        AISystem() = default;
        ~AISystem() = default;

        /**
         * \brief This is used to initialize the AI and set the GameDataModel reference and the amount of sets of cards to store
         * \param gameDataModel
         * \param amountOfPlaysToStore
         */
        void Initialize(GameDataDatabase* gameDataModel, size_t amountOfPlaysToStore = 3);

        /**
         * \brief This function is used to run game simulations and generate balancing information
         */
        void RunBalancingTool(MatchData& matchData);

        /**
         * \brief This function is used to make the AI select a card using the given match data and player ID
         * \param matchData 
         * \param aiPlayerID 
         * \return Card ID
         */
        unsigned int RunGameAI(MatchData& matchData, unsigned int aiPlayerID);

        /**
         * \brief This function is used to Simulate a game with the given cards
         * \param simulationMatchData
         * \return It returns the cards played and the utility scores attached to these cards
         */
        void SimulateGame(MatchData& simulationMatchData);


    public:

    private:
        /**
         * \brief This function picks the 4 cards to place on the slots
         * \param simulationMatchData
         * \param playerID
         * \return ptl::vector<unsigned int>
         */
        ptl::vector<tbsg::PlayedCard> PickCards(MatchData& simulationMatchData, unsigned int playerID);

        PlayedCard SimulateTurn(MatchData& simulationMatch, unsigned int playerID, unsigned int selectedCard);

        ptl::vector<PlayedCard> GetTopCards(ptl::vector<PlayedCard>& playedCards, MatchData& simulationMatchData);

        unsigned int GetBestCard(ptl::vector<PlayedCard>& topCards);

        void LoadUtilityFormulas();

        float CalculateUtility(MatchData& matchDummy, MatchData& originalMatchData, unsigned int playerID);

        /**
         * \brief This function is used to add multiple scores and get the average score
         * \param utilityScores
         * \return float
         */
        float AddUtilityScores(ptl::vector<float>& utilityScores);

        void PrintDeck(const ptl::vector<unsigned int>& deck, const ptl::string& deckType);

        void GetChangeData(const ResultOfRound& results, ptl::vector<PlayerChanges>& playerChanges);

        void StoreGatheredGameData(const GameInfo& gameInfo);

        void AddTurnDataToJson(rapidjson::Document& document, rapidjson::Value& turnsArray, const ptl::vector<TurnInfo>& turnInfo);
        void AddMatchDataToJson(rapidjson::Document& document, rapidjson::Value& cardObject, const MatchData& matchData, ptl::string matchObjectName);
        void AddMatchDataToJson(rapidjson::Document& document, rapidjson::Value& cardObject, const MatchData& matchData, int playerIndex, ptl::string matchObjectName);
        void AddHeroDataToJson(rapidjson::Document& document, rapidjson::Value& matchObject, const MatchData& matchData, int playerIndex, ptl::string playerObjectName);

        void AddPlayerDataToJson(rapidjson::Document& document, rapidjson::Value& playerArray, const PlayerChanges& playerChanges);
        void AddPlayerDataToJson(rapidjson::Document& document, rapidjson::Value& playerArray, const PlayerChanges& playerChanges, ptl::string objectName);

    private:
        tbsg::GameDataDatabase* gameDataModelRef = nullptr;

        size_t numberOfPlaysToStore = 3;

        std::vector<std::pair<std::function<float(float)>, float>> utilityFunctions;

        bool limitTurnAmount = true;

    };
    /*! @} */
}

#include "model/AISystem.h"
#include "gameplay/DataStructs.h"
#include "gameplay/scripting/LuaContext.h"
#include "Payloads.h"
#include "GameDataDatabase.h"
#include "gameplay/MatchHelper.h"
#include "catch/catch.hpp"

#include <rapidjson/document.h>
#include "core/Config.h"
#include <rapidjson/filereadstream.h>
#include "gameplay/GamePlayHelper.h"
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include "LoggingFunction.h"
#include "core/Assertion.h"

#ifndef errno_t
typedef int errno_t;
#endif

#ifdef __unix
#define fopen_s(pFile,filename,mode) ((*(pFile))=fopen((filename),  (mode)))==NULL
#endif

void tbsg::AISystem::Initialize(GameDataDatabase* gameDataModel, size_t amountOfPlaysToStore)
{
    LoadUtilityFormulas();

    gameDataModelRef = gameDataModel;
    numberOfPlaysToStore = amountOfPlaysToStore;
}

void tbsg::AISystem::RunBalancingTool(MatchData& matchData)
{
	gameplay::ShuffleCards(matchData.playerDecks[0]);
	gameplay::ShuffleCards(matchData.playerDecks[1]);

    if(matchData.playerHands[0].empty())
    {
        tbsg::gameplay::DrawCards(matchData.playerDecks[0], matchData.playerHands[0], 4);
    }
    if (matchData.playerHands[1].empty())
    {
        tbsg::gameplay::DrawCards(matchData.playerDecks[1], matchData.playerHands[1], 4);
    }

    SimulateGame(matchData);
    //std::exit(1);
}

unsigned tbsg::AISystem::RunGameAI(MatchData& matchData, unsigned int aiPlayerID)
{
    unsigned int selectedCard = 0;
    ptl::vector<PlayedCard> cardsAI = PickCards(matchData, aiPlayerID);
    selectedCard = GetBestCard(cardsAI);
    if(selectedCard == 0)
    {
        cof::Error("No card was selected by the AI!");
        if (matchData.playerHands[aiPlayerID].empty())
        {
            cof::Error("AI Player's hand was empty!");
        }
        else
        {
            PrintDeck(matchData.playerHands[aiPlayerID], "Hand");
        }
        PrintDeck(matchData.playerDecks[aiPlayerID], "Deck");
        PrintDeck(matchData.playerDiscards[aiPlayerID], "Discard");
        //TODO: handle when no card is in selected
    }

    return selectedCard;
}

void tbsg::AISystem::SimulateGame(MatchData& simulationMatchData)
{
	//cof::Info("SimulateGame:");
    GameInfo gameInfo;
    scripting::LuaContext luaContext;
    luaContext.Initialize();

    int turnCounter = 0;
    //As long as the Heroes are alive, the game should be simulated
    while(!simulationMatchData.isMatchDone && (turnCounter < 1000 || !limitTurnAmount))
    {

        ++turnCounter;
		cof::Info("Turn {}", turnCounter);
        unsigned int firstCard = 0;
        unsigned int secondCard = 0;
		//cof::Info("PickCards:");
        ptl::vector<PlayedCard> cardsFirstAI = PickCards(simulationMatchData, 0);
        ptl::vector<PlayedCard> cardsSecondAI = PickCards(simulationMatchData, 1);

        if (simulationMatchData.heroes[0].health > 0)
        {
            firstCard = GetBestCard(cardsFirstAI);
        }
        else
        {
            Card tempCard;
            tempCard.meta.name = "No card selected";
            for (auto& playedCard : cardsFirstAI)
            {
                playedCard.selectedCard = tempCard;
            }
        }

        if (simulationMatchData.heroes[1].health > 0)
        {
            secondCard = GetBestCard(cardsSecondAI);
        }
        else
        {
            Card tempCard;
            tempCard.meta.name = "No card selected";
            for (auto& playedCard : cardsSecondAI)
            {
                playedCard.selectedCard = tempCard;
            }
        }
		//cof::Info("Play game {}", turnCounter);
        simulationMatchData.playedCards = ptl::vector<unsigned int>{firstCard, secondCard};
        ResultOfRound result = gameplay::OnPlay(simulationMatchData, luaContext, gameDataModelRef);

        ptl::vector<PlayerChanges> changes { 2 };
        GetChangeData(result, changes);

        TurnInfo turnInfo{ simulationMatchData, std::make_pair(cardsFirstAI, cardsSecondAI), changes};
        gameInfo.turnInfo.push_back(turnInfo);
    }
    StoreGatheredGameData(gameInfo);
    gameInfo.turnInfo.clear();
    luaContext.Deallocate();
}

ptl::vector<tbsg::PlayedCard> tbsg::AISystem::PickCards(MatchData& simulationMatchData, unsigned int playerID)
{
    ptl::vector<std::future<PlayedCard>> futures;
    ptl::vector<tbsg::PlayedCard> simulationResults;


    ////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////// !!! This will run the simulations without async !!! /////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////
    //////simulationResults.push_back(SimulateTurn(simulationMatchData, playerID, 0));            //
    //for (auto& selectedCard : simulationMatchData.playerHands[playerID])                     //
    //{                                                                                       //
    //    PlayedCard result = SimulateTurn(simulationMatchData, playerID, selectedCard);      //
    //    simulationResults.push_back(result);                                                //
    //}                                                                                       //
    ////////////////////////////////////////////////////////////////////////////////////////////

    // Add one simulation for no card played.
	//futures.push_back(std::async(std::launch::async, [&] { return AISystem::SimulateTurn(simulationMatchData, playerID, 0); }));
    
    assert(!simulationMatchData.playerHands[playerID].empty() && "Hand is empty should not be possible");

    // Add simulations for each card possible
    for (auto& selectedCard : simulationMatchData.playerHands[playerID])
    {
		futures.push_back(std::async(std::launch::async, [&] { return AISystem::SimulateTurn(simulationMatchData, playerID, selectedCard); }));
    }
    
    for (auto& currentFuture : futures)
    {
        if (!currentFuture.valid()) { continue; }
        PlayedCard result = currentFuture.get();
        simulationResults.push_back(result);
    }
    futures.clear();

	auto bestCards = GetTopCards(simulationResults, simulationMatchData);
    
    return bestCards;
}


tbsg::PlayedCard tbsg::AISystem::SimulateTurn(MatchData& simulationMatch, unsigned int playerID, unsigned int selectedCard)
{
    MatchData dummyMatch = simulationMatch;
    PlayedCard cardInfo;
    scripting::LuaContext luaContext;

    luaContext.Initialize();

    dummyMatch.playedCards[playerID] = selectedCard;
    dummyMatch.playedCards[!playerID] = 0;

    if(selectedCard != 0)
    {
        cardInfo.selectedCard = *gameDataModelRef->GetCard(selectedCard);
    }
    else
    {
        Card emptyCard;
        emptyCard.meta.name = "No Card Played";
        emptyCard.id = 0;
    
        cardInfo.selectedCard = emptyCard;
    }
    cardInfo.matchDataBefore = dummyMatch;
    const ResultOfRound results = gameplay::OnPlay(dummyMatch, luaContext, gameDataModelRef);

    GetChangeData(results, cardInfo.playerChanges);
    
    cardInfo.matchDataAfter = dummyMatch;
    cardInfo.utilityScore = CalculateUtility(dummyMatch, simulationMatch, playerID);

    luaContext.Deallocate();
	return cardInfo;
}

ptl::vector<tbsg::PlayedCard> tbsg::AISystem::GetTopCards(ptl::vector<PlayedCard>& playedCards, MatchData& simulationMatchData)
{
    ptl::vector<PlayedCard> topCards;
    float lowestUtilityScore    = 1.f;

    for (auto& currentCard : playedCards)
    {
        if (topCards.size() >= numberOfPlaysToStore)
        {
            if (currentCard.utilityScore > lowestUtilityScore)
            {
                for (auto& currentTopCard : topCards)
                {
                    if (currentTopCard.utilityScore == lowestUtilityScore)
                    {
                        currentTopCard = currentCard;
                        break;
                    }
                }
                for (auto& currentTopCard : topCards)
                {
                    lowestUtilityScore = 1.f;
                    if (currentTopCard.utilityScore < lowestUtilityScore)
                    {
                        lowestUtilityScore = currentTopCard.utilityScore;
                    }
                }
            }
        }
        else
        {
            topCards.push_back(currentCard);
            if (currentCard.utilityScore < lowestUtilityScore)
            {
                lowestUtilityScore = currentCard.utilityScore;
            }
        }
    }
    return topCards;
}

unsigned tbsg::AISystem::GetBestCard(ptl::vector<PlayedCard>& topCards)
{
    float highestUtilityScore = 0.f;
    PlayedCard* bestCard = nullptr;

    for(auto& currentCard : topCards)
    {
        if(currentCard.utilityScore >= highestUtilityScore)
        {
            highestUtilityScore = currentCard.utilityScore;
            bestCard = &currentCard;
        }
    }
    if(bestCard)
    {  
        bestCard->isCardPlayed = true;
        return bestCard->selectedCard.id;
    }

    return 0;
}

float tbsg::AISystem::CalculateUtility(MatchData& matchDummy, MatchData& originalMatchData, unsigned int playerID)
{
    ptl::vector<float> utilityScores;
    utilityScores.reserve(5);
    
    utilityScores.push_back(0.5f);

    const unsigned int opponentID = !playerID;


    if (matchDummy.playedCards[playerID] == 0)
    {
        utilityScores.push_back(0.0f);
        utilityScores.push_back(0.25f);
    }
    if (matchDummy.playedCards[playerID] == 0)
    {
        utilityScores.push_back(0.0f);
        utilityScores.push_back(0.25f);
    }

    //Check difference in the stats of the AI Hero
    // CurrentPlayerHealth
    int statDifference = matchDummy.heroes[playerID].health - originalMatchData.heroes[playerID].health;
    if (statDifference != 0)
    {
        if (matchDummy.heroes[playerID].health <= 0)
        {
            utilityScores.emplace_back(0.f);
            utilityScores.emplace_back(0.f);
            utilityScores.emplace_back(0.f);
        }
        statDifference += utilityFunctions[0].second * 0.5;
        utilityScores.push_back(utilityFunctions[0].first(static_cast<float>(statDifference)));
    }

    ////CurrentPlayerAttack
    //statDifference = a_MatchDummy.heroes[playerID].attack - originalMatchData.heroes[playerID].attack;
    //if (statDifference != 0)
    //{
    //    statDifference += utilityFunctions[3].second * 0.5;
    //    utilityScores.push_back(utilityFunctions[3].first(static_cast<float>(statDifference)));
    //}

    //CurrentPlayerArmor
    statDifference = matchDummy.heroes[playerID].armor - originalMatchData.heroes[playerID].armor;
    if (statDifference != 0)
    {
        statDifference += utilityFunctions[5].second * 0.5;
        utilityScores.push_back(utilityFunctions[5].first(static_cast<float>(statDifference)));
    }

    ////CurrentPlayerResource
    //statDifference = a_MatchDummy.heroes[playerID].resource - originalMatchData.heroes[playerID].resource;
    //if (statDifference != 0)
    //{
    //    statDifference += utilityFunctions[8].second * 0.5;
    //    utilityScores.push_back(utilityFunctions[8].first(static_cast<float>(statDifference)));
    //}

    //CurrentPlayerHand
    size_t cardDifference = matchDummy.playerHands[playerID].size() - originalMatchData.playerHands[playerID].size();
    if (cardDifference != 0)
    {
        cardDifference += utilityFunctions[10].second * 0.5;
        utilityScores.push_back(utilityFunctions[10].first(static_cast<float>(cardDifference)));
    }

    //CurrentPlayerDeck
    cardDifference = matchDummy.playerDecks[playerID].size() - originalMatchData.playerDecks[playerID].size();
    if (cardDifference != 0)
    {
        cardDifference += utilityFunctions[12].second * 0.5;
        utilityScores.push_back(utilityFunctions[12].first(static_cast<float>(cardDifference)));
    }

    // These commented functions are for game data which is not used in our current game, but are still supported in the gameplay code
    ////CurrentPlayerWeapon
    //if (a_MatchDummy.heroes[playerID].weapon != nullptr)
    //{
    //    if(originalMatchData.heroes[playerID].weapon != nullptr)
    //    {
    //        statDifference = a_MatchDummy.heroes[playerID].weapon->attack - originalMatchData.heroes[playerID].weapon->attack;
    //    }
    //    else
    //    {
    //        statDifference = a_MatchDummy.heroes[playerID].weapon->attack;
    //    }
    //    if (statDifference != 0)
    //    {
    //        statDifference += utilityFunctions[14].second * 0.5;
    //        utilityScores.push_back(utilityFunctions[14].first(static_cast<float>(statDifference)));
    //    }
    //
    //    if (originalMatchData.heroes[playerID].weapon != nullptr)
    //    {
    //        statDifference = a_MatchDummy.heroes[playerID].weapon->durability - originalMatchData.heroes[playerID].weapon->durability;
    //    }
    //    else
    //    {
    //        statDifference = a_MatchDummy.heroes[playerID].weapon->durability;
    //    }
    //    if (statDifference != 0)
    //    {
    //        statDifference += utilityFunctions[16].second * 0.5;
    //        utilityScores.push_back(utilityFunctions[16].first(static_cast<float>(statDifference)));
    //    }
    //}

    //Check difference in the stats of the Rival Hero
    //OpponentHeath
    statDifference = matchDummy.heroes[opponentID].health - originalMatchData.heroes[opponentID].health;
    if (statDifference != 0)
    {
        statDifference += utilityFunctions[1].second * 0.5;
        utilityScores.push_back(utilityFunctions[1].first(static_cast<float>(statDifference)));
    }

    ////OpponentAttack
    //statDifference = a_MatchDummy.heroes[opponentID].attack - originalMatchData.heroes[opponentID].attack;
    //if (statDifference != 0)
    //{
    //    statDifference += utilityFunctions[4].second * 0.5;
    //    utilityScores.push_back(utilityFunctions[4].first(static_cast<float>(statDifference)));
    //}

    //OpponentArmor
    statDifference = matchDummy.heroes[opponentID].armor - originalMatchData.heroes[opponentID].armor;
    if (statDifference != 0)
    {
        statDifference += utilityFunctions[6].second * 0.5;
        utilityScores.push_back(utilityFunctions[6].first(static_cast<float>(statDifference)));
    }

    ////OpponentResource
    //statDifference = a_MatchDummy.heroes[opponentID].resource - originalMatchData.heroes[opponentID].resource;
    //if (statDifference != 0)
    //{
    //    statDifference += utilityFunctions[9].second * 0.5;
    //    utilityScores.push_back(utilityFunctions[9].first(static_cast<float>(statDifference)));
    //}

    //OpponentHand
    cardDifference = matchDummy.playerHands[opponentID].size() - originalMatchData.playerHands[opponentID].size();
    if (cardDifference != 0)
    {
        cardDifference += utilityFunctions[11].second * 0.5;
        utilityScores.push_back(utilityFunctions[11].first(static_cast<float>(cardDifference)));
    }

    //OpponentDeck
    cardDifference = matchDummy.playerDecks[opponentID].size() - originalMatchData.playerDecks[opponentID].size();
    if (cardDifference != 0)
    {
        cardDifference += utilityFunctions[13].second * 0.5;
        utilityScores.push_back(utilityFunctions[13].first(static_cast<float>(cardDifference)));
    }

    ////OpponentWeapon
    //if (originalMatchData.heroes[opponentID].weapon)
    //{
    //    if (originalMatchData.heroes[opponentID].weapon != nullptr)
    //    {
    //        statDifference = a_MatchDummy.heroes[opponentID].weapon->attack - originalMatchData.heroes[opponentID].weapon->attack;
    //    }
    //    else
    //    {
    //        statDifference = a_MatchDummy.heroes[opponentID].weapon->attack;
    //    }
    //    if (statDifference != 0)
    //    {
    //        statDifference += utilityFunctions[15].second * 0.5;
    //        utilityScores.push_back(utilityFunctions[15].first(static_cast<float>(statDifference)));
    //    }
    //
    //    if (originalMatchData.heroes[opponentID].weapon != nullptr)
    //    {
    //        statDifference = a_MatchDummy.heroes[opponentID].weapon->durability - originalMatchData.heroes[opponentID].weapon->durability;
    //    }
    //    else
    //    {
    //        statDifference = a_MatchDummy.heroes[opponentID].weapon->durability;
    //    }
    //    if (statDifference != 0)
    //    {
    //        statDifference += utilityFunctions[17].second * 0.5;
    //        utilityScores.push_back(utilityFunctions[17].first(static_cast<float>(statDifference)));
    //    }
    //}

    // Utility functions for the monster stats
    //MonsterHealth
    if(!matchDummy.monsterCards.empty())
    {
        if(matchDummy.monsterCards.size() == originalMatchData.monsterCards.size())
        {
            statDifference = matchDummy.monsterCards[0].data.health - originalMatchData.monsterCards[0].data.health;
            if(statDifference != 0)
            {
                statDifference += utilityFunctions[2].second * 0.5;
                utilityScores.push_back(utilityFunctions[2].first(static_cast<float>(statDifference)));
            }

            statDifference = matchDummy.monsterCards[0].data.armor - originalMatchData.monsterCards[0].data.armor;
            if (statDifference != 0)
            {
                statDifference += utilityFunctions[7].second * 0.5;
                utilityScores.push_back(utilityFunctions[7].first(static_cast<float>(statDifference)));
            }
        }
        else
        {
            utilityScores.push_back(1.f);
        }
    }

    return AddUtilityScores(utilityScores);
}

void tbsg::AISystem::LoadUtilityFormulas()
{
    rapidjson::GenericDocument< rapidjson::UTF8<>, rapidjson::MemoryPoolAllocator<> > aiDoc;
    char readBuffer[512];

    FILE* fp;
    errno_t err = fopen_s(&fp, tbsg::Config::Get().aiFile.c_str(), "r");
    ASSERT_MSG(!err, "Could not load config");


    rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    aiDoc.ParseStream(is);

    assert(aiDoc.IsObject());

    for (auto& StatObject : aiDoc.GetObject())
    {
        assert(StatObject.value.IsObject());
        assert(StatObject.value["AI"].IsObject());
        for (auto& HeroObject : StatObject.value.GetObject())
        {
            assert(HeroObject.value.IsObject());

            assert(HeroObject.value["GraphType"].IsInt());
            assert(HeroObject.value["Arguments"].IsArray());

            const int graphType = HeroObject.value["GraphType"].GetInt();
            switch (graphType)
            {
            case 0:
            {
                assert(HeroObject.value["Arguments"].GetArray().Size() > 1);
                assert(HeroObject.value["Arguments"][0].IsBool());
                assert(HeroObject.value["Arguments"][1].IsFloat());

                bool shouldGraphFlip = HeroObject.value["Arguments"][0].GetBool();
                float maxRange = HeroObject.value["Arguments"][1].GetFloat();

                // This Lambda calculates the Utility according to a linear function
                const std::function<float(float)> function = // Lambda:
                    [maxRange, shouldGraphFlip]
                (float a_Variable) -> float
                {
                    float utilityScore = a_Variable / maxRange;

                    // Utility should not be higher than one
                    if (utilityScore > 1.f)
                    {
                        utilityScore = 1.f;
                    }

                    // Flip means to flip the line upside down.
                    if (shouldGraphFlip)
                    {
                        utilityScore = 1 - utilityScore;
                    }

                    return utilityScore;
                };

                utilityFunctions.emplace_back(function, maxRange);
                break;
            }
            case 1:
            {
                assert(HeroObject.value["Arguments"].GetArray().Size() > 1);
                assert(HeroObject.value["Arguments"][0].IsBool());
                assert(HeroObject.value["Arguments"][1].IsFloat());
                assert(HeroObject.value["Arguments"][2].IsFloat());

                bool shouldGraphFlip = HeroObject.value["Arguments"][0].GetBool();
                float maxRange = HeroObject.value["Arguments"][1].GetFloat();
                float steepness = HeroObject.value["Arguments"][2].GetFloat();

                // This Lambda calculates the Utility according to a quadratic function
                const std::function<float(float)> function = // Lambda:
                    [maxRange, shouldGraphFlip, steepness]
                (float a_Variable) -> float
                {
                    float utilityScore = powf(a_Variable / maxRange, steepness);

                    // Utility should not be higher than one
                    if (utilityScore > 1.f)
                    {
                        utilityScore = 1.f;
                    }

                    // Flip means to flip the curve upside down.
                    if (shouldGraphFlip)
                    {
                        utilityScore = 1 - utilityScore;
                    }

                    return utilityScore;
                };

                utilityFunctions.emplace_back(function, maxRange);
                break;
            }
            case 2:
            {
                assert(HeroObject.value["Arguments"].GetArray().Size() > 1);
                assert(HeroObject.value["Arguments"][0].IsBool());
                assert(HeroObject.value["Arguments"][1].IsFloat());
                assert(HeroObject.value["Arguments"][2].IsFloat());

                bool shouldGraphFlip = HeroObject.value["Arguments"][0].GetBool();
                float maxRange = HeroObject.value["Arguments"][1].GetFloat();
                float steepness = HeroObject.value["Arguments"][2].GetFloat();

                // This Lambda calculates the Utility according to a sigmoid curve function
                const std::function<float(float)> function =
                    [maxRange, shouldGraphFlip, steepness]
                (float a_Variable) -> float
                {
                    float utilityScore = 1 / (1 + powf(steepness, -(a_Variable / maxRange * 12) + 6));

                    // 1 / (1 + s ^ (-(x / r * 12) + 6))

                    // Utility should not be higher than one
                    if (utilityScore > 1.f)
                    {
                        utilityScore = 1.f;
                    }

                    // Flip means to flip the curve upside down.
                    if (shouldGraphFlip)
                    {
                        utilityScore = 1 - utilityScore;
                    }

                    return utilityScore;
                };

                utilityFunctions.emplace_back(function, maxRange);
                break;
            }
            case 3:
            {
                assert(HeroObject.value["Arguments"].GetArray().Size() > 1);
                assert(HeroObject.value["Arguments"][0].IsFloat());
                assert(HeroObject.value["Arguments"][1].IsFloat());
                assert(HeroObject.value["Arguments"][2].IsFloat());
                assert(HeroObject.value["Arguments"][3].IsFloat());
                assert(HeroObject.value["Arguments"][4].IsFloat());
                assert(HeroObject.value["Arguments"][5].IsFloat());
                assert(HeroObject.value["Arguments"][6].IsFloat());

                float startingHeight = HeroObject.value["Arguments"][0].GetFloat();
                float height1 = HeroObject.value["Arguments"][1].GetFloat();
                float height2 = HeroObject.value["Arguments"][2].GetFloat();
                float height3 = HeroObject.value["Arguments"][3].GetFloat();
                float height4 = HeroObject.value["Arguments"][4].GetFloat();
                float height5 = HeroObject.value["Arguments"][5].GetFloat();
                float maxRange = HeroObject.value["Arguments"][6].GetFloat();

                // This Lambda calculates the Utility according to a Piecewise linear line function
                const std::function<float(float)> function =
                    [startingHeight, height1, height2, height3, height4, height5, maxRange]
                (float a_Variable) -> float
                {
                    float utilityScore = 0.f;
                    const float range = maxRange / 5;

                    // This function divides the range over 5 pieces which each have their own linear function

                    if (a_Variable < range)                     // First piece
                    {
                        float s = (height1 - startingHeight) / range;

                        utilityScore = s * a_Variable + startingHeight;
                    }
                    else if (a_Variable < range * 2)   // Second piece
                    {
                        float s = (height2 - height1) / range;
                        float x = a_Variable - range;

                        utilityScore = s * x + height1;
                    }
                    else if (a_Variable < range * 3)            // Third piece
                    {
                        float s = (height3 - height2) / range;
                        float x = a_Variable - range * 2;

                        utilityScore = s * x + height2;
                    }
                    else if (a_Variable < range * 4)            // Fourth piece
                    {
                        float s = (height4 - height3) / range;
                        float x = a_Variable - range * 3;

                        utilityScore = s * x + height3;
                    }
                    else if (a_Variable <= maxRange)          // Fifth piece
                    {
                        float s = (height5 - height4) / range;
                        float x = a_Variable - range * 4;

                        utilityScore = s * x + height4;
                    }
                    else
                    {
                        utilityScore = 1.f;
                    }

                    // Utility should not be higher than one
                    if (utilityScore > 1.f)
                    {
                        utilityScore = height5;
                    }
                    else if (utilityScore < 0.f)
                    {
                        utilityScore = startingHeight;
                    }

                    return utilityScore;
                };

                utilityFunctions.push_back(std::make_pair(function, maxRange));
                break;
            }
            default:
            {
                //ASSERT_MSG(false, "One of the selected graph types does not exist [ai.json]")
                throw std::runtime_error(&"One of the selected graph types does not exist, selected graph type is:"[graphType]);
                break;
            }
            }

        }
    }
    fclose(fp);
}

// This function is used to add multiple scores and get the average score
float tbsg::AISystem::AddUtilityScores(ptl::vector<float>& utilityScores)
{
    float sumUtilityScores      = 0.f;
    int positiveScoreCounter    = 0;
    int negativeScoreCounter    = 0;

    for (float utilityScore : utilityScores)
    {
        sumUtilityScores += utilityScore;

        if(utilityScore > 0.55f)
        {
            ++positiveScoreCounter;
        }
        else if(utilityScore < 0.45f)
        {
            ++negativeScoreCounter;
        }
    }

    // The effectCountScore is used to balance having multiple effects, this means that a card has multiple effects that are good, 
    // it will get an additional score added to the overall utility score. So that a card with multiple positive effects is more likely
    // to be played then a card with only one positive effect.
    const float effectCountScore = (positiveScoreCounter - negativeScoreCounter) / 100.f;
    float result = sumUtilityScores / utilityScores.size() + effectCountScore;

    // Normalize the value again.
    if(result > 1.f)
    {
        result = 1.f;
    }
    else if (result < 0.f)
    {
        result = 0.f;
    }

    return result;
}

void tbsg::AISystem::PrintDeck(const ptl::vector<unsigned>& deck, const ptl::string& deckType)
{
    ptl::string gameInfo;
    for (auto& card : deck)
    {
        gameInfo = deckType + ": Card = \"" + gameDataModelRef->GetCard(card)->meta.name + "\" ID = \"" + ptl::to_string(card);
        cof::Info(gameInfo.c_str());
    }
    std::cout << "\n\n";
}

void tbsg::AISystem::GetChangeData(const ResultOfRound& results, ptl::vector<PlayerChanges>& playerChanges)
{
    for (auto& currentChange : results.results)
    {
        const int index = currentChange.index;
        switch (currentChange.changeType)
        {
            case EffectChange::Hero_Health :
            {
                if (currentChange.change < 0)
                {
                    playerChanges[index].damageTaken += -currentChange.change;
                }
                else
                {
                    playerChanges[index].healingDone += currentChange.change;
                }
                break;
            }
            case EffectChange::Hero_Armor :
            {
                if(currentChange.change < 0)
                {
                    playerChanges[index].armorLost += -currentChange.change;
                }
                else
                {
                    playerChanges[index].armorGained += currentChange.change;
                }
                break;
            }
            default :
            {
                break;
            }
        }
    }
}

void tbsg::AISystem::StoreGatheredGameData(const GameInfo& gameInfo)
{
    rapidjson::Document simulationDataDoc;
    simulationDataDoc.Parse("{}");

    // Add the data gathered from each turn to the turns array
    rapidjson::Value turnsArray;
    turnsArray.SetArray();
    turnsArray.Reserve(static_cast<rapidjson::SizeType>(gameInfo.turnInfo.size()), simulationDataDoc.GetAllocator());
    AddTurnDataToJson(simulationDataDoc, turnsArray, gameInfo.turnInfo);

    //simulationDataDoc["game"].SetObject();
    simulationDataDoc.AddMember("seed", gameInfo.turnInfo.front().playedCards.first.front().matchDataBefore.seed, simulationDataDoc.GetAllocator());
    
    const int totalMonsterAmount  = gameInfo.turnInfo.front().playedCards.first.front().matchDataAfter.monsterCards.size();
    int monstersLeft = 0;
    if(!gameInfo.turnInfo.back().playedCards.first.front().matchDataAfter.monsterCards.empty())
    {
        monstersLeft = gameInfo.turnInfo.back().playedCards.first.front().matchDataAfter.monsterCards.size();
    }

    simulationDataDoc.AddMember("monsterskilled", totalMonsterAmount - monstersLeft, simulationDataDoc.GetAllocator());
    simulationDataDoc.AddMember("turns", turnsArray, simulationDataDoc.GetAllocator());

    std::time_t t = std::time(nullptr);

    auto path = Config::Get().GetBalancingData() + "SimulationData-" + ptl::to_string(t) + ".json";
    FILE* fp;
    errno_t err = fopen_s(&fp, path.c_str(), "w+");

    if(err)
    {
        cof::Error("Could not write to \"SimulationData.json\", make sure this file is writable!!!");
        ASSERT_MSG(!err, "Could not load config");;
    }

    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    simulationDataDoc.Accept(writer);

    const char* output = buffer.GetString();

    fwrite(buffer.GetString(), sizeof(char), buffer.GetLength(), fp);
    fclose(fp);
    cof::Info("Data Of Simulated Game Is Stored!");
}

void tbsg::AISystem::AddTurnDataToJson(rapidjson::Document& document, rapidjson::Value& turnsArray,
    const ptl::vector<TurnInfo>& turnInfo)
{
    int turnCounter = 0;
    // Loop over all the plays made by the AI players in the game
    for (auto& turn : turnInfo)
    {
        ++turnCounter;

        rapidjson::Value turnObject;
        turnObject.SetObject();
        turnObject.MemberReserve(2, document.GetAllocator());

        turnObject.AddMember("turn", turnCounter, document.GetAllocator());

        for(auto& currentCard : turn.playedCards.first)
        {
            if(currentCard.isCardPlayed)
            {
                turnObject.AddMember("encounterid", turnInfo.front().playedCards.first.front().matchDataBefore.monsterCards.size() - 
                                      currentCard.matchDataAfter.monsterCards.size() + 1, document.GetAllocator());

                break;
            }
        }

        AddMatchDataToJson(document, turnObject, turn.matchDataAfter, "matchdataaftercard");
        AddPlayerDataToJson(document, turnObject, turn.playerChanges[0], "player1changes");
        AddPlayerDataToJson(document, turnObject, turn.playerChanges[1], "player2changes");

        // Add the top (3) best cards of the first player to the turnObject
        rapidjson::Value playerOneBestCards;
        playerOneBestCards.SetArray();
        playerOneBestCards.Reserve(static_cast<rapidjson::SizeType>(turn.playedCards.first.size()), document.GetAllocator());

        // Go over all the cards of the first player
        for (auto& currentCardInfo : turn.playedCards.first)
        {
            // Make a card object
            rapidjson::Value cardObject;
            cardObject.SetObject();
            cardObject.MemberReserve(4, document.GetAllocator());

            // The name of the card that was selected and the utility score that the AI gave to the card
            cardObject.AddMember("cardid", currentCardInfo.selectedCard.id, document.GetAllocator());
            cardObject.AddMember("cardname", rapidjson::StringRef(currentCardInfo.selectedCard.meta.name.c_str()), document.GetAllocator());
            cardObject.AddMember("utilityscore", currentCardInfo.utilityScore, document.GetAllocator());
            cardObject.AddMember("iscardplayed", currentCardInfo.isCardPlayed, document.GetAllocator());

            AddPlayerDataToJson(document, cardObject, currentCardInfo.playerChanges[0]);

            AddMatchDataToJson(document, cardObject, currentCardInfo.matchDataBefore, 0, "matchdatabefore");
            AddMatchDataToJson(document, cardObject, currentCardInfo.matchDataAfter, 0, "matchdataafter");

            playerOneBestCards.PushBack(cardObject, document.GetAllocator());
        }

        turnObject.AddMember("player1bestcards", playerOneBestCards, document.GetAllocator());

        // Go over all the cards of the second player
        // Add the top (3) best cards of the second player to the turnObject
        rapidjson::Value playerTwoBestCards;
        playerTwoBestCards.SetArray();
        playerTwoBestCards.Reserve(static_cast<rapidjson::SizeType>(turn.playedCards.first.size()), document.GetAllocator());

        for (auto& currentCardInfo : turn.playedCards.second)
        {
            // Make a card object
            rapidjson::Value cardObject;
            cardObject.SetObject();
            cardObject.MemberReserve(6, document.GetAllocator());

            // The name of the card that was selected and the utility score that the AI gave to the card
            cardObject.AddMember("cardid", currentCardInfo.selectedCard.id, document.GetAllocator());
            cardObject.AddMember("cardname", rapidjson::StringRef(currentCardInfo.selectedCard.meta.name.c_str()), document.GetAllocator());
            cardObject.AddMember("utilityscore", currentCardInfo.utilityScore, document.GetAllocator());
            cardObject.AddMember("iscardplayed", currentCardInfo.isCardPlayed, document.GetAllocator());

            AddPlayerDataToJson(document, cardObject, currentCardInfo.playerChanges[1]);

            AddMatchDataToJson(document, cardObject, currentCardInfo.matchDataBefore, 1, "matchdatabefore");
            AddMatchDataToJson(document, cardObject, currentCardInfo.matchDataAfter, 1, "matchdataafter");

            playerTwoBestCards.PushBack(cardObject, document.GetAllocator());
        }
        turnObject.AddMember("player2bestcards", playerTwoBestCards, document.GetAllocator());

        turnsArray.PushBack(turnObject, document.GetAllocator());
    }
}

void tbsg::AISystem::AddMatchDataToJson(rapidjson::Document& document, rapidjson::Value& cardObject, const MatchData& matchData, ptl::string matchObjectName)
{
    // Make an "before" object for data from before the match 
    rapidjson::Value matchObject;
    matchObject.SetObject();
    matchObject.MemberReserve(4, document.GetAllocator());

    //Add the matchdata before the card was played to the json (e.g. health, armor, attack)
    rapidjson::Value objectName(rapidjson::kStringType);
    objectName.SetString(matchObjectName.c_str(), strlen(matchObjectName.c_str()), document.GetAllocator());

    AddHeroDataToJson(document, matchObject, matchData, 0, "player1");
    AddHeroDataToJson(document, matchObject, matchData, 1, "player2");

    rapidjson::Value monsterData;
    monsterData.SetObject();
    monsterData.MemberReserve(5, document.GetAllocator());

    if(!matchData.monsterCards.empty())
    {
        monsterData.AddMember("monsteralive", true, document.GetAllocator());
        monsterData.AddMember("monsterID", matchData.monsterCards.front().id, document.GetAllocator());
        monsterData.AddMember("monstername", rapidjson::StringRef(matchData.monsterCards.front().meta.name.c_str()), document.GetAllocator());
        monsterData.AddMember("monsterhealth", matchData.monsterCards.front().data.health, document.GetAllocator());
        monsterData.AddMember("monsterarmor", matchData.monsterCards.front().data.armor, document.GetAllocator());
    }
    else
    {
        monsterData.AddMember("MonsterAlive", false, document.GetAllocator());
    }

    matchObject.AddMember("monster", monsterData, document.GetAllocator());


    cardObject.AddMember(objectName, matchObject, document.GetAllocator());
}

void tbsg::AISystem::AddMatchDataToJson(rapidjson::Document& document, rapidjson::Value& cardObject, const MatchData& matchData, int playerIndex, ptl::string matchObjectName)
{
    // Make an "before" object for data from before the match 
    rapidjson::Value matchObject;
    matchObject.SetObject();
    matchObject.MemberReserve(2, document.GetAllocator());

    //Add the matchdata before the card was played to the json (e.g. health, armor, attack)
    rapidjson::Value objectName(rapidjson::kStringType);
    objectName.SetString(matchObjectName.c_str(), strlen(matchObjectName.c_str()), document.GetAllocator());

    AddHeroDataToJson(document, matchObject, matchData, playerIndex, "player");
    AddHeroDataToJson(document, matchObject, matchData, !playerIndex, "opponent");

    cardObject.AddMember(objectName, matchObject, document.GetAllocator());
}

void tbsg::AISystem::AddHeroDataToJson(rapidjson::Document& document, rapidjson::Value& matchObject, const MatchData& matchData, int playerIndex, ptl::string playerObjectName)
{
    // Make a playerObject to store all the player data
    rapidjson::Value playerObject;
    playerObject.SetObject();
    playerObject.MemberReserve(9, document.GetAllocator());

    // Add player data
    playerObject.AddMember("health",        matchData.heroes[playerIndex].health,           document.GetAllocator());
    playerObject.AddMember("armor",         matchData.heroes[playerIndex].armor,            document.GetAllocator());
    playerObject.AddMember("decksize",      matchData.playerDecks[playerIndex].size(),      document.GetAllocator());
    playerObject.AddMember("discardsize",   matchData.playerDiscards[playerIndex].size(),   document.GetAllocator());
    playerObject.AddMember("handsize",      matchData.playerHands[playerIndex].size(),      document.GetAllocator());

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // THIS SHOULD NOT BE REMOVED!!! This is commented because it is currently not used, but might be in the future! //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //playerObject.AddMember("maxHealth",     heroInfo.maxHealth,     document.GetAllocator());
    //playerObject.AddMember("attack",        heroInfo.attack,        document.GetAllocator());                      //
    //playerObject.AddMember("baseAttack",    heroInfo.baseAttack,    document.GetAllocator());                      //
    //playerObject.AddMember("resource",      heroInfo.resource,      document.GetAllocator());                      //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    rapidjson::Value objectName(rapidjson::kStringType);
    objectName.SetString(playerObjectName.c_str(), strlen(playerObjectName.c_str()), document.GetAllocator());

    matchObject.AddMember(objectName, playerObject, document.GetAllocator());
}

void tbsg::AISystem::AddPlayerDataToJson(rapidjson::Document& document, rapidjson::Value& playerArray, const PlayerChanges& playerChanges)
{
    rapidjson::Value playerObject;
    playerObject.SetObject();
    playerObject.MemberReserve(4, document.GetAllocator());

    playerObject.AddMember("healthgained",  playerChanges.healingDone,  document.GetAllocator());
    playerObject.AddMember("healthlost",    playerChanges.damageTaken,  document.GetAllocator());
    playerObject.AddMember("armorgained",   playerChanges.armorGained,  document.GetAllocator());
    playerObject.AddMember("armorlost",     playerChanges.armorLost,    document.GetAllocator());

    playerArray.AddMember("playerchanges", playerObject, document.GetAllocator());
}

void tbsg::AISystem::AddPlayerDataToJson(rapidjson::Document& document, rapidjson::Value& playerArray, const PlayerChanges& playerChanges, ptl::string objectName)
{
    rapidjson::Value playerObject;
    playerObject.SetObject();
    playerObject.MemberReserve(4, document.GetAllocator());

    playerObject.AddMember("healthgained", playerChanges.healingDone, document.GetAllocator());
    playerObject.AddMember("healthlost", playerChanges.damageTaken, document.GetAllocator());
    playerObject.AddMember("armorgained", playerChanges.armorGained, document.GetAllocator());
    playerObject.AddMember("armorlost", playerChanges.armorLost, document.GetAllocator());

    rapidjson::Value memberName(rapidjson::kStringType);
    memberName.SetString(objectName.c_str(), strlen(objectName.c_str()), document.GetAllocator());

    playerArray.AddMember(memberName, playerObject, document.GetAllocator());
}

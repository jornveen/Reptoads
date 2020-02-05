//#define CATCH_CONFIG_MAIN
//
//#include "catch/catch.hpp"
//
//#include "scripting/LuaBindingFunc.h"
//#include "gameplay/scripting/LuaContext.h"
//#include "Payloads.h"
//#include "gameplay/scripting/LuaContextHelper.h"
//
//#include "model/GameDataModel.h"
//#include "core/StrHash.h"
//#include "core/Config.h"
//#include "resource_loading/ConfigLoader.h"
//#include "Logger.h"
//#include "model/AISystem.h"
//
//tbsg::scripting::LuaContext m_LuaContext;
//tbsg::Config config;
//tbsg::GameDataDatabase m_GameDataModel{ m_LuaContext };
//tbsg::MatchData match;
//
//
//const tbsg::Card GetCard(std::string a_Name)
//{
//    const tbsg::Card* card = m_GameDataModel.GetCard(tbsg::SimpleHash(a_Name.c_str()));;
//    return *card;
//}
//
//void GiveReward(tbsg::Hero * a_Hero, ptl::vector<tbsg::Reward*>& a_Reward)
//{
//    for (const auto reward : a_Reward)
//    {
//
//        switch (reward->type)
//        {
//            case tbsg::CardRewardType::CardRewardType_None:
//            {
//                break;
//            }
//
//            case tbsg::CardRewardType::CardRewardType_Armor:
//            {
//                a_Hero->armor += reward->powerup;
//                break;
//            }
//
//            case tbsg::CardRewardType::CardRewardType_Attack:
//            {
//                a_Hero->attack += reward->powerup;
//                break;
//            }
//
//            case tbsg::CardRewardType::CardRewardType_Resource:
//            {
//                a_Hero->resource += reward->powerup;
//                break;
//            }
//
//            case tbsg::CardRewardType::CardRewardType_Weapon:
//            {
//                if (reward->weapon != nullptr)
//                {
//                    a_Hero->weapon = reward->weapon;
//                }
//                break;
//            }
//
//            case tbsg::CardRewardType::CardRewardType_Health:
//            {
//                a_Hero->health += reward->powerup;
//                break;
//            }
//        }
//    }
//}
//
//tbsg::Hero GetHero()
//{
//    tbsg::Hero hero;
//    hero.armor = 30;
//    hero.attack = 10;
//    hero.health = 30;
//    hero.resource = 10;
//    tbsg::Weapon* weapon = new tbsg::Weapon();
//    weapon->attack = 10;
//    weapon->durability = 10;
//    hero.weapon = weapon;
//    return hero;
//}
//
//tbsg::MatchData GetMatch()
//{
//    tbsg::MatchData match;
//    match.heros[0] = GetHero();
//    match.heros[1] = GetHero();
//
//    match.decks[0] = *m_GameDataModel.GetDeck("Deck1");
//    match.decks[1] = *m_GameDataModel.GetDeck("Deck1");
//
//    match.hand[0].push_back(m_GameDataModel.GetCard(tbsg::SimpleHash("Anticipation")));
//    match.hand[0].push_back(m_GameDataModel.GetCard(tbsg::SimpleHash("Anticipation")));
//    match.hand[0].push_back(m_GameDataModel.GetCard(tbsg::SimpleHash("Anticipation")));
//    match.hand[0].push_back(m_GameDataModel.GetCard(tbsg::SimpleHash("Anticipation")));
//
//    match.hand[1].push_back(m_GameDataModel.GetCard(tbsg::SimpleHash("FishingTrawler")));
//    match.hand[1].push_back(m_GameDataModel.GetCard(tbsg::SimpleHash("Anticipation")));
//    match.hand[1].push_back(m_GameDataModel.GetCard(tbsg::SimpleHash("Anticipation")));
//    match.hand[1].push_back(m_GameDataModel.GetCard(tbsg::SimpleHash("Anticipation")));
//
//
//    match.played_cards[0][0] = tbsg::SimpleHash("Enfeeble");
//    match.played_cards[0][1] = tbsg::SimpleHash("Enfeeble");
//    match.played_cards[0][2] = tbsg::SimpleHash("AlKharidWarrior");
//    match.played_cards[0][3] = tbsg::SimpleHash("AlKharidWarrior");
//
//    match.played_cards[1][0] = tbsg::SimpleHash("AlKharidWarrior");
//    match.played_cards[1][1] = tbsg::SimpleHash("AlKharidWarrior");
//    match.played_cards[1][2] = tbsg::SimpleHash("Enfeeble");
//    match.played_cards[1][3] = tbsg::SimpleHash("Enfeeble");
//    return match; 
//}
//
//void FillMatchCards(tbsg::MatchData& match, ptl::vector<ptl::vector<tbsg::Card>>& vec)
//{
//    ptl::vector<tbsg::Card> tempvec;
//    vec.reserve(match.played_cards.size());
//    for (size_t i = 0; i < match.played_cards.size(); ++i)
//    {
//        vec.push_back(tempvec);
//        vec[i].reserve(match.played_cards[i].size());
//        for (size_t j = 0; j < match.played_cards[i].size(); ++j)
//        {
//            if (match.played_cards[i][j] != 0)
//            {
//                const tbsg::Card* ccard = m_GameDataModel.GetCard(match.played_cards[i][j]);
//                tbsg::Card card = *ccard;
//                vec[i].push_back(card);
//            }
//        }
//    }
//}
//
//tbsg::MatchData GetCardResult(std::string a_CardName, tbsg::MatchData a_Match)
//{
//    tbsg::MatchData* match = &a_Match;
//
//    tbsg::Card card = GetCard(a_CardName);
//    tbsg::Card* test = &card;
//
//    tbsg::Script script = *m_GameDataModel.GetScript(tbsg::SimpleHash(test->meta.name.c_str()));
//    sol::safe_function_result scriptCode = m_LuaContext.RunScript(script.code);
//
//    int currentPlayerID = 0;
//    int currentCardPlayedIndex = 0;
//    bool isHeroDead = false;
//
//    ptl::vector<ptl::vector<tbsg::Card>> matchCards;
//    matchCards = ptl::vector<ptl::vector<tbsg::Card>>();
//    FillMatchCards(*match, matchCards);
//
//    tbsg::ResultOfRound resultOfRound{};
//
//    for (size_t i = 0; i < 8; ++i)
//    {
//        resultOfRound.playedCards.push_back(0);
//        resultOfRound.results.push_back(ptl::vector<tbsg::Change>{});
//    }
//
//
//    if (m_LuaContext.IsValid(scriptCode)) {
//
//        tbsg::scripting::BindMatchFunc(m_LuaContext, match, currentPlayerID, currentCardPlayedIndex, matchCards, &m_GameDataModel, resultOfRound, currentCardPlayedIndex);
//        tbsg::scripting::BindPlayerFunc(m_LuaContext,match, resultOfRound, currentCardPlayedIndex, isHeroDead);
//        tbsg::scripting::BindDeckFunc(m_LuaContext);
//        tbsg::scripting::BindCardFunc(m_LuaContext, match, matchCards, resultOfRound, currentCardPlayedIndex);
//
//        sol::protected_function func;
//        m_LuaContext.GetFunction(func, "OnCardPlay");
//
//        std::function<void(tbsg::Card*)> OnCardPlay = func;
//        OnCardPlay(test);
//        GiveReward(&match->heros[0], test->data.reward);
//    }
//
//    return *match;
//}
//
//ptl::vector<ptl::vector<tbsg::Card>> GetMatchCardResult(std::string a_CardName, tbsg::MatchData a_Match)
//{
//    tbsg::MatchData* match = &a_Match;
//
//    tbsg::Card card = GetCard(a_CardName);
//    tbsg::Card* test = &card;
//
//    tbsg::Script script = *m_GameDataModel.GetScript(tbsg::SimpleHash(test->meta.name.c_str()));
//    sol::safe_function_result scriptCode = m_LuaContext.RunScript(script.code);
//
//    int currentPlayerID = 0;
//    int currentCardPlayedIndex = 0;
//    bool isHeroDead = false;
//
//    ptl::vector<ptl::vector<tbsg::Card>> matchCards;
//    matchCards = ptl::vector<ptl::vector<tbsg::Card>>();
//    FillMatchCards(*match, matchCards);
//
//    tbsg::ResultOfRound resultOfRound{};
//
//    for (size_t i = 0; i < 8; ++i)
//    {
//        resultOfRound.playedCards.push_back(0);
//        resultOfRound.results.push_back(ptl::vector<tbsg::Change>{});
//    }
//
//
//    if (m_LuaContext.IsValid(scriptCode)) {
//
//        tbsg::scripting::BindMatchFunc(m_LuaContext, match, currentPlayerID, currentCardPlayedIndex, matchCards, &m_GameDataModel, resultOfRound, currentCardPlayedIndex);
//        tbsg::scripting::BindPlayerFunc(m_LuaContext, match, resultOfRound, currentCardPlayedIndex, isHeroDead);
//        tbsg::scripting::BindDeckFunc(m_LuaContext);
//        tbsg::scripting::BindCardFunc(m_LuaContext, match, matchCards, resultOfRound, currentCardPlayedIndex);
//
//        sol::protected_function func;
//        m_LuaContext.GetFunction(func, "OnCardPlay");
//
//        std::function<void(tbsg::Card*)> OnCardPlay = func;
//        OnCardPlay(test);
//        GiveReward(&match->heros[0], test->data.reward);
//    }
//
//    return matchCards;
//}
//
//bool Init()
//{
//    cof::basic_logger::InitStdOut();
//    tbsg::LoadConfig("./config.json");
//    dataLoader.Load();
//    dataLoader.LoadProfiles("users");
//    dataLoader.LoadProfiles("bots");
//    m_GameDataModel.Initialize(dataLoader);
//    m_LuaContext.Initialize();
//
//    return true;
//}
//
//TEST_CASE("Run Init", "[multi-file:2]")
//{
//    REQUIRE(Init() == true);
//}
//
///*
//// test for the switching lane feature.
//TEST_CASE("Test for TestScript", "[multi-file:2]")
//{
//    REQUIRE(GetMatchCardResult("TestScript", GetMatch())[0][1].meta.name == "AlKharidWarrior");
//    REQUIRE(GetMatchCardResult("TestScript", GetMatch())[0][2].meta.name == "Enfeeble");
//    REQUIRE(GetMatchCardResult("TestScript", GetMatch())[0][3].meta.name == "Enfeeble");
//
//    REQUIRE(GetMatchCardResult("TestScript", GetMatch())[1][1].meta.name == "Enfeeble");
//    REQUIRE(GetMatchCardResult("TestScript", GetMatch())[1][2].meta.name == "AlKharidWarrior");
//    REQUIRE(GetMatchCardResult("TestScript", GetMatch())[1][3].meta.name == "AlKharidWarrior");
//}*/
//
//
//
///*
//TEST_CASE("Test card AlKharidWarrior", "[multi-file:2]")
//{
//	REQUIRE(GetCardResult("AlKharidWarrior", GetMatch()).heros[0].resource == 13);
//	REQUIRE(GetCardResult("AlKharidWarrior", GetMatch()).heros[1].resource == 8);
//}
//
//TEST_CASE("Test card Enfeeble", "[multi-file:2]")
//{
//	REQUIRE(GetCardResult("Enfeeble", GetMatch()).hand[0].size() == 3);
//	REQUIRE(GetMatchCardResult("Enfeeble", GetMatch())[0][2].data.attack == 0);
//}
//
//TEST_CASE("Test card Banshee", "[multi-file:2]")
//{
//    REQUIRE(GetCardResult("Banshee", GetMatch()).heros[0].resource == 11);
//    REQUIRE(GetMatchCardResult("Banshee", GetMatch())[0][3].data.attack == 1);
//    REQUIRE(GetMatchCardResult("Banshee", GetMatch())[0][2].data.health == 4);
//    REQUIRE(GetMatchCardResult("Banshee", GetMatch())[0][2].data.attack == 1);
//    REQUIRE(GetMatchCardResult("Banshee", GetMatch())[0][3].data.health == 4);
//}
//
//TEST_CASE("Test card QueenOfMusic", "[multi-file:2]")
//{
//	REQUIRE(GetCardResult("QueenOfMusic", match).heros[1].health == 32);
//}
//
//TEST_CASE("Test card WindStrike", "[multi-file:2]")
//{
//	REQUIRE(GetCardResult("WindStrike", GetMatch()).heros[1].health == 27);
//}
//
//TEST_CASE("Test card CaveSlime", "[multi-file:2]")
//{
//	REQUIRE(GetCardResult("CaveSlime", GetMatch()).heros[0].health == 30);
//	REQUIRE(GetMatchCardResult("CaveSlime", GetMatch())[0][2].data.health == 2);
//}
//
//TEST_CASE("Test card Char", "[multi-file:2]")
//{
//	REQUIRE(GetCardResult("Char", GetMatch()).heros[0].attack == 15);
//	REQUIRE(GetCardResult("Char", GetMatch()).heros[0].weapon == nullptr);
//}
//TEST_CASE("Test card BlackChinchompa", "[multi-file:2]")
//{
//	REQUIRE(GetCardResult("BlackChinchompa", GetMatch()).heros[0].armor == 32);
//	REQUIRE(GetCardResult("BlackChinchompa", GetMatch()).heros[1].weapon->durability == 9);
//}
//
//TEST_CASE("Test card ChaosDwogre", "[multi-file:2]")
//{
//	REQUIRE(GetCardResult("ChaosDwogre", GetMatch()).heros[0].resource == 12);
//	REQUIRE(GetCardResult("ChaosDwogre", GetMatch()).heros[0].armor == 34);
//	REQUIRE(GetCardResult("ChaosDwogre", GetMatch()).heros[0].weapon->durability == 10);
//}
//
//TEST_CASE("Test card Scorpion", "[multi-file:2]")
//{
//	REQUIRE(GetCardResult("Scorpion", GetMatch()).heros[1].armor == 27);
//}
//
//TEST_CASE("Test card CustomsOfficer", "[multi-file:2]")
//{
//	REQUIRE(GetCardResult("CustomsOfficer", GetMatch()).heros[0].resource == 16);
//}
//
//TEST_CASE("Test card CrazedWhaler", "[multi-file:2]")
//{
//	REQUIRE(GetCardResult("CrazedWhaler", GetMatch()).heros[0].resource == 12);
//	REQUIRE(GetCardResult("CrazedWhaler", GetMatch()).heros[1].resource == 8);
//	REQUIRE(GetCardResult("CrazedWhaler", GetMatch()).heros[1].weapon->durability == 8);
//}
//
//TEST_CASE("Test card Bunny", "[multi-file:2]")
//{
//	REQUIRE(GetCardResult("Bunny", GetMatch()).heros[0].resource == 11);
//	REQUIRE(GetCardResult("Bunny", GetMatch()).heros[0].health == 31);
//}
//
//TEST_CASE("Test card TrollChucker", "[multi-file:2]")
//{
//	REQUIRE(GetCardResult("TrollChucker", GetMatch()).heros[0].resource == 11);
//	REQUIRE(GetCardResult("TrollChucker", GetMatch()).heros[1].armor == 26);
//}
//
//TEST_CASE("Test card HauntedSoul", "[multi-file:2]")
//{
//	REQUIRE(GetCardResult("HauntedSoul", GetMatch()).heros[0].resource == 12);
//	REQUIRE(GetCardResult("HauntedSoul", GetMatch()).heros[1].armor == 25);
//}
//
//TEST_CASE("Test card InfernalMage", "[multi-file:2]")
//{
//	REQUIRE(GetCardResult("InfernalMage", GetMatch()).heros[0].resource == 13);
//	REQUIRE(GetCardResult("InfernalMage", GetMatch()).heros[1].health == 24);
//}
//
//TEST_CASE("Test card UndeadDragon", "[multi-file:2]")
//{
//    REQUIRE(GetCardResult("UndeadDragon", GetMatch()).heros[0].resource == 12);
//    REQUIRE(GetCardResult("UndeadDragon", GetMatch()).heros[1].health == 20);
//}
//
//*/
//
//TEST_CASE("Test card BobtheCat", "[multi-file:2]")
//{
//    REQUIRE(GetCardResult("BobtheCat", GetMatch()).heros[0].armor == 10);
//    REQUIRE(GetCardResult("BobtheCat", GetMatch()).heros[0].resource == 13);
//    REQUIRE(GetCardResult("BobtheCat", GetMatch()).heros[1].resource == 7);
//}
//
//TEST_CASE("Test card ChaoticLongsword", "[multi-file:2]")
//{
//    REQUIRE(GetCardResult("ChaoticLongsword", GetMatch()).heros[0].weapon->attack == 6);
//    REQUIRE(GetCardResult("ChaoticLongsword", GetMatch()).heros[0].weapon->durability == 2);
//}
//
//TEST_CASE("Test card Doric", "[multi-file:2]")
//{
//    REQUIRE(GetCardResult("Doric", GetMatch()).heros[0].armor == 34);
//    REQUIRE(GetCardResult("Doric", GetMatch()).heros[1].armor == 26);
//}
//
//
//TEST_CASE("Test card DragonLongsword", "[multi-file:2]")
//{
//    REQUIRE(GetCardResult("DragonLongsword", GetMatch()).heros[0].weapon->attack == 5);
//    REQUIRE(GetCardResult("DragonLongsword", GetMatch()).heros[0].weapon->durability == 3);
//}
//
//TEST_CASE("Test card Hunger", "[multi-file:2]")
//{
//    REQUIRE(GetCardResult("Hunger", GetMatch()).heros[0].health == 25);
//    REQUIRE(GetCardResult("Hunger", GetMatch()).heros[0].attack == 11);
//}
//
//TEST_CASE("Test card PreparingtoHunt", "[multi-file:2]")
//{
//    REQUIRE(GetCardResult("PreparingtoHunt", GetMatch()).heros[0].armor == 35);
//    REQUIRE(GetCardResult("PreparingtoHunt", GetMatch()).heros[0].weapon->attack == 2);
//    REQUIRE(GetCardResult("PreparingtoHunt", GetMatch()).heros[0].weapon->durability == 2);
//    REQUIRE(GetCardResult("PreparingtoHunt", GetMatch()).hand[0].size() == 5);
//}
//
//TEST_CASE("Test card RestorePotion", "[multi-file:2]")
//{
//    REQUIRE(GetCardResult("RestorePotion", GetMatch()).heros[0].health == 35);
//    REQUIRE(GetCardResult("RestorePotion", GetMatch()).hand[0].size() == 6);
//}
//
//TEST_CASE("Test card SteelTitan", "[multi-file:2]")
//{
//    REQUIRE(GetCardResult("SteelTitan", GetMatch()).heros[0].attack == 11);
//    REQUIRE(GetCardResult("SteelTitan", GetMatch()).heros[1].armor == 25);
//}
//
//TEST_CASE("Test card ZulrahsScales", "[multi-file:2]")
//{
//    REQUIRE(GetCardResult("ZulrahsScales", GetMatch()).heros[0].resource == 18);
//    REQUIRE(GetCardResult("ZulrahsScales", GetMatch()).heros[0].health == 26);
//}
//
//
//
//TEST_CASE("Test card AlKharidWarrior", "[multi-file:2]")
//{
//    REQUIRE(GetCardResult("AlKharidWarrior", GetMatch()).heros[0].resource == 13);
//    REQUIRE(GetCardResult("AlKharidWarrior", GetMatch()).heros[1].resource == 8);
//}
//
//TEST_CASE("Test card AnimatedArmour", "[multi-file:2]")
//{
//    REQUIRE(GetCardResult("AnimatedArmour", GetMatch()).heros[0].armor == 50);
//    REQUIRE(GetCardResult("AnimatedArmour", GetMatch()).heros[0].resource == 12);
//}
//
//TEST_CASE("Test card DrunkenPirate", "[multi-file:2]")
//{
//    REQUIRE(GetCardResult("DrunkenPirate", GetMatch()).heros[0].resource == 11);
//    REQUIRE(GetCardResult("DrunkenPirate", GetMatch()).heros[1].resource == 9);
//}
//
//TEST_CASE("Test card FlyingLeech", "[multi-file:2]")
//{
//    REQUIRE(GetCardResult("FlyingLeech", GetMatch()).heros[0].health == 32);
//    REQUIRE(GetCardResult("FlyingLeech", GetMatch()).heros[1].health == 28);
//}
//
//TEST_CASE("Test card Jerrod", "[multi-file:2]")
//{
//    REQUIRE(GetCardResult("Jerrod", GetMatch()).heros[0].armor == 26);
//    REQUIRE(GetCardResult("Jerrod", GetMatch()).heros[1].armor == 26);
//}
//
//TEST_CASE("Test card KalphiteSoldier", "[multi-file:2]")
//{
//    REQUIRE(GetCardResult("KalphiteSoldier", GetMatch()).heros[0].armor == 32);
//    REQUIRE(GetCardResult("KalphiteSoldier", GetMatch()).heros[0].resource == 13);
//}
//
//TEST_CASE("AI tests", "[multi-file:2]")
//{
//    tbsg::AISystem ai;
//    ai.Initialize(&m_GameDataModel, 1);
//
//    //Linear Function Test
//    {
//        float maxRange = 50;
//        bool shouldGraphFlip = false;
//
//        const std::function<float(float)> function = // Lambda:
//            [maxRange, shouldGraphFlip]
//        (float a_Variable) -> float
//        {
//            float utilityScore = a_Variable / maxRange;
//
//            // Utility should not be higher than one
//            if (utilityScore > 1.f)
//            {
//                utilityScore = 1.f;
//            }
//
//            // Flip means to flip the line upside down.
//            if (shouldGraphFlip)
//            {
//                utilityScore = 1 - utilityScore;
//            }
//
//            return utilityScore;
//        };
//        REQUIRE(function(15) == 0.3f);
//    }
//
//    //Quadratic Function Test
//    {
//        float maxRange = 50;
//        float steepness = 3;
//        bool shouldGraphFlip = false;
//
//        const std::function<float(float)> function = // Lambda:
//            [maxRange, shouldGraphFlip, steepness]
//        (float a_Variable) -> float
//        {
//            float utilityScore = powf(a_Variable / maxRange, steepness);
//
//            // Utility should not be higher than one
//            if (utilityScore > 1.f)
//            {
//                utilityScore = 1.f;
//            }
//
//            // Flip means to flip the curve upside down.
//            if (shouldGraphFlip)
//            {
//                utilityScore = 1 - utilityScore;
//            }
//
//            return utilityScore;
//        };
//
//        REQUIRE(std::abs(function(33) - (35937.f / 125000.f)) < 0.01);
//    }
//
//    //Sigmoid Function Test
//    {
//        bool shouldGraphFlip = false;
//        float maxRange = 500;
//        float steepness = 2;
//        // This Lambda calculates the Utility according to a sigmoid curve function
//        const std::function<float(float)> function =
//            [maxRange, shouldGraphFlip, steepness]
//        (float a_Variable) -> float
//        {
//            float utilityScore = 1 / (1 + powf(steepness, -(a_Variable / maxRange * 12) + 6));
//
//            // 1 / (1 + s ^ (-(x / r * 12) + 6))
//
//            // Utility should not be higher than one
//            if (utilityScore > 1.f)
//            {
//                utilityScore = 1.f;
//            }
//
//            // Flip means to flip the curve upside down.
//            if (shouldGraphFlip)
//            {
//                utilityScore = 1 - utilityScore;
//            }
//
//            return utilityScore;
//        };
//
//        REQUIRE(std::abs(function(125) - (1.f / 9.f)) < 0.01);
//    }
//
//    //Piecewise Function Test
//    {
//        float startingHeight = 0.6f;
//        float height1 = 0.3f;
//        float height2 = 0.7f;
//        float height3 = 0.55f;
//        float height4 = 0.1f;
//        float height5 = 0.4f;
//        float maxRange = 20.f;
//
//        // This Lambda calculates the Utility according to a Piecewise linear line function
//        const std::function<float(float)> function =
//            [startingHeight, height1, height2, height3, height4, height5, maxRange]
//        (float a_Variable) -> float
//        {
//            float utilityScore = 0.f;
//            const float range = maxRange / 5;
//
//            // This function divides the range over 5 pieces which each have their own linear function
//
//            if (a_Variable < range)                     // First piece
//            {
//                float s = (height1 - startingHeight) / range;
//
//                utilityScore = s * a_Variable + startingHeight;
//            }
//            else if (a_Variable < range * 2)   // Second piece
//            {
//                float s = (height2 - height1) / range;
//                float x = a_Variable - range;
//
//                utilityScore = s * x + height1;
//            }
//            else if (a_Variable < range * 3)            // Third piece
//            {
//                float s = (height3 - height2) / range;
//                float x = a_Variable - range * 2;
//
//                utilityScore = s * x + height2;
//            }
//            else if (a_Variable < range * 4)            // Fourth piece
//            {
//                float s = (height4 - height3) / range;
//                float x = a_Variable - range * 3;
//
//                utilityScore = s * x + height3;
//            }
//            else if (a_Variable <= maxRange)          // Fifth piece
//            {
//                float s = (height5 - height4) / range;
//                float x = a_Variable - range * 4;
//
//                utilityScore = s * x + height4;
//            }
//            else
//            {
//                utilityScore = height5;
//            }
//
//            // Utility should not be higher than one
//            if (utilityScore > 1.f)
//            {
//                utilityScore = 1.f;
//            }
//            else if (utilityScore < 0.f)
//            {
//                utilityScore = 0.f;
//            }
//
//            return utilityScore;
//        };
//
//        REQUIRE(function(3) == 0.375f);
//        REQUIRE(function(5) == 0.4f);
//        REQUIRE(function(10) == 0.625f);
//        REQUIRE(function(15) == 0.2125f);
//        REQUIRE(std::abs(function(19) - 0.325f) < 0.01);
//        REQUIRE(function(30) == 0.4f);
//
//    }
//
//    // Test AI card picking functions
//    {
//        tbsg::MatchData testMatch;
//
//        testMatch.heros[1].resource = 100;
//
//        while (!testMatch.hand[1].empty())
//        {
//            testMatch.hand[1].pop_back();
//        }
//
//        std::string name = "BobtheCat";
//        tbsg::Card tempcard = *m_GameDataModel.GetCard(tbsg::SimpleHash(name.c_str()));
//        testMatch.hand[1].push_back(&tempcard);
//
//        name = "DrunkenPirate";
//        tbsg::Card tempcard2 = *m_GameDataModel.GetCard(tbsg::SimpleHash(name.c_str()));
//        testMatch.hand[1].push_back(&tempcard2);
//
//        ai.m_MatchRef = &testMatch;
//
//        // Test the SimulateGame function
//        // Warning: Only works with the expected cards (BobtheCat and DrunkenPirate
//        {
//            tbsg::CardSetOpt result = ai.SimulateGame(ptl::vector<tbsg::Card>{tempcard, tempcard2}, 0, 1);
//            REQUIRE(result.UtilityScore == 0.45f);
//        }
//
//        // Test card picking system, 
//        // Warning: Only works with the expected cards (BobtheCat and DrunkenPirate)
//        {
//            ptl::vector<unsigned int> result = ai.PickCards(&testMatch);
//            ptl::vector<unsigned int> expectedResult = { tbsg::SimpleHash("BobtheCat"), tbsg::SimpleHash("DrunkenPirate"), 0, 0 };
//
//            REQUIRE(result == expectedResult);
//        }
//
//        ai.m_MatchRef = nullptr;
//    }
//}
//
///* Not used yet
//TEST_CASE("Test for First page support cards", "[multi-file:2]")
//{
//    REQUIRE(GetCardResult("50ShipsMufassah", GetMatch()).heros[0].resource == 15);
//
//    REQUIRE(GetCardResult("AbyssalWhip", GetMatch()).heros[0].resource == 2);
//    REQUIRE(GetCardResult("AdamantArmour", GetMatch()).heros[0].resource == 2);
//    REQUIRE(GetCardResult("Alchemy", GetMatch()).heros[0].resource == 2);
//    REQUIRE(GetCardResult("AliMorrisane", GetMatch()).heros[0].resource == 2);
//    REQUIRE(GetCardResult("AmascutRuins", GetMatch()).heros[0].resource == 2);
//    REQUIRE(GetCardResult("Ambush", GetMatch()).heros[0].resource == 2);
//    REQUIRE(GetCardResult("AmbushedHive", GetMatch()).heros[0].resource == 2);
//    REQUIRE(GetCardResult("AmuletofFury", GetMatch()).heros[0].resource == 2);
//    REQUIRE(GetCardResult("AncientSpellbook", GetMatch()).heros[0].resource == 2);
//    REQUIRE(GetCardResult("AngerofDelrith", GetMatch()).heros[0].resource == 2);
//    REQUIRE(GetCardResult("Anticipation", GetMatch()).heros[0].resource == 2);
//    REQUIRE(GetCardResult("AntifirePotion", GetMatch()).heros[0].resource == 2);
//    REQUIRE(GetCardResult("ArchmageSedridor", GetMatch()).heros[0].resource == 2);
//    REQUIRE(GetCardResult("ArtisansWorkshop", GetMatch()).heros[0].resource == 2);
//    REQUIRE(GetCardResult("Asphyxiate", GetMatch()).heros[0].resource == 2);
//    REQUIRE(GetCardResult("Assault", GetMatch()).heros[0].resource == 2);
//    REQUIRE(GetCardResult("BanditCamp", GetMatch()).heros[0].resource == 2);
//    REQUIRE(GetCardResult("BarFight", GetMatch()).heros[0].resource == 2);
//    REQUIRE(GetCardResult("BarbarianHonour", GetMatch()).heros[0].resource == 2);
//}*/
#include <algorithm>
#include <numeric>
#include <catch/catch.hpp>
#include "gameplay/MatchHelper.h"
#include "gameplay/DataStructs.h"
#include "gameplay/scripting/LuaContext.h"

TEST_CASE("[Engine-Gameplay] Test calling Lua script")
{
	tbsg::scripting::LuaContext lua;
	lua.Initialize();

	//Test case from Sol2 documents adjusted by using provided helpers
	int x = 0;

	std::function<void()> beepFunc = [&x]() {++x; };
	lua.SetFunction("beep", beepFunc);

	tbsg::Script script;
	script.code = "beep()";

	CHECK(tbsg::gameplay::RunLuaScript(lua, script) == true);
	CHECK(x == 1);

	sol::protected_function beep = lua.GetFunction("beep");
	beep();
	CHECK(x == 2);
}

TEST_CASE("[Engine-Gameplay] Perform card pile manipulations")
{
	tbsg::MatchData match;
	ptl::vector<unsigned int> testInts(30, 0);
	match.playerDeck[0] = testInts;
	match.playerDeck[1] = testInts;

	//Fill with random numbers
	std::iota(match.playerDeck[0].begin(), match.playerDeck[0].end(), tbsg::gameplay::GetRandomIntInRange(1, 100000));
	std::iota(match.playerDeck[1].begin(), match.playerDeck[1].end(), tbsg::gameplay::GetRandomIntInRange(1, 100000));

	CHECK((match.playerDeck[0][0] != 0 && match.playerDeck[1][0] != 0));

	//Switch the cards around based on amount generated and total limits.
	unsigned int playerIndex = 0;
	unsigned int selectionIndex = 0;
	unsigned int amountCardsToGet = 0;
	size_t previousSize = 0;
	for (int i = 0; i < 100; ++i)
	{
		playerIndex = tbsg::gameplay::GetRandomIntInRange(0, 1);
		selectionIndex = tbsg::gameplay::GetRandomIntInRange(0, 2);
		amountCardsToGet = tbsg::gameplay::GetRandomIntInRange(0, 4);

		switch (selectionIndex)
		{
		case 0:
		{
			tbsg::gameplay::DrawCards(match.playerDeck[playerIndex], match.playerHand[playerIndex], amountCardsToGet, 6);
			CHECK(match.playerHand[playerIndex].size() <= 6);
			break;
		}
		case 1:
		{
			tbsg::gameplay::DrawCards(match.playerHand[playerIndex], match.playerDiscard[playerIndex], amountCardsToGet, 30);
			CHECK(match.playerDiscard[playerIndex].size() <= 30);
			break;
		}
		case 2:
		{
			tbsg::gameplay::DrawCards(match.playerDiscard[playerIndex], match.playerDeck[playerIndex], amountCardsToGet, 30);
			CHECK(match.playerDeck[playerIndex].size() <= 30);
			break;
		}
		}
	}
}
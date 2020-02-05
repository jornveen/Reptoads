#pragma once
#include "Payloads.h"
#include <random>

namespace tbsg{
    /**
     * \brief representation of the current game (the whole game)
     */
    struct MatchData {


		//Card Information of the map
		ptl::vector<tbsg::MonsterCard> monsterCards{1};

		//Card Information of each player
		ptl::vector<ptl::vector<unsigned int>> playerDecks{2};
		ptl::vector<ptl::vector<unsigned int>> playerHands{2};
		ptl::vector<ptl::vector<unsigned int>> playerDiscards{2};

        //Heroes
        ptl::vector<tbsg::Hero> heroes{ 2 };

		//Match Played
		ptl::vector<unsigned int> playedCards{0,0};

        //Amount of turns player is in fight with the monster. Resets when it goes to the next monster
        unsigned int playerFightDuration;

		//Match States
		unsigned int playerReadyState{ 0 };
		unsigned int nextStartingPlayer{ 0 };
        unsigned int playerAmount{ 2 };

		std::mt19937 gameSeed{0};
		unsigned int seed{ 0 };

        bool isMatchDone{false};
    };
}


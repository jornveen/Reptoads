#include "utils/CMDInput.h"
#include "Net/Server.h"
#include "RTSARG.h"
#include "GameServer.h"
#include "core/StrHash.h"
#include "GameDataDatabase.h"
#include "model/GameLogic.h"

#include <cxxopts.hpp>
#include <iostream>
#include <mutex>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif
#include "LoggingFunction.h"
#include "gameplay/MatchHelper.h"
#include "Net/Packet.h"

void ClearScreen()
{
#ifdef _WIN32

    HANDLE hStdOut;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD count;
    DWORD cellCount;
    COORD homeCoords = {0, 0};

    hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStdOut == INVALID_HANDLE_VALUE)
        return;

    /* Get the number of cells in the current buffer */
    if (!GetConsoleScreenBufferInfo(hStdOut, &csbi))
        return;
    cellCount = csbi.dwSize.X * csbi.dwSize.Y;

    /* Fill the entire buffer with spaces */
    if (!FillConsoleOutputCharacter(hStdOut, (TCHAR)' ', cellCount, homeCoords, &count))
        return;

    /* Fill the entire buffer with the current colors and attributes */
    if (!FillConsoleOutputAttribute(hStdOut, csbi.wAttributes, cellCount, homeCoords, &count))
        return;

    /* Move the cursor home */
    SetConsoleCursorPosition(hStdOut, homeCoords);
#else
	std::cout << "\033[2J\033[1;1H";

#endif
}

void CMDInput::ReadCin(std::atomic<bool> &run, std::mutex &mutex, std::vector<std::string> &msgQueue)
{
    while (run.load()) {
        std::string buffer;
        std::getline(std::cin, buffer);
        if (buffer == "quit") {
            run.store(false);
        }
        else if (buffer == "clear") {
            ClearScreen();
        }
        else {
            std::lock_guard<std::mutex> lock_guard(mutex);
            msgQueue.push_back(buffer);
        }
    }
}

void CMDInput::HandleCommands(RTSARG &args, tbsg::GameServer* server)
{
    try {
        char **argv = args.GetArgv();
        int argc = args.GetArgc();

        if (argc > 0) {
            std::string baseCommand(args.GetArgv()[0]);

            if (baseCommand == "help") {
                printf("Commands:\n");
                printf("quit\tstops the server\n");
                printf("help\tdisplays this info\n");
                printf("clear\tclears screen\n");
                printf("debug, --on, --off\ttoggle the server to print client commands\n");
                printf("lobby\n\t-l,--list\tlist all lobbies.\n\t-s,--stop\tstops the specified lobby.\n\t-g,--ingame\tsets the ingame state of the lobby\n");
                printf("clients\n\t-l,--list\t lists all clients\n");
                printf("profiles\n\t-l,--list\t lists all profiles the system knows\n\t");
                std::cout << "-g,--get\t get profile by profile information name \n\t"
                          << "-n,--name\t gets the basic information about the profile. Example:-g -n UserNamex64 \n\t"
                          << "-c,--cards\t gets a list of all the cards the player owns.\n";
                printf("profile\n\t-a,--a,-p,-player,-b,-bot,-n,--name\t Add player/bot\n\t");
                printf("matches\n\t-l,--list\t lists all matches the system knows\n\t\n");
                printf("match\n\t-s,--simulate\t Simulate game with cards\n\t");
            }
            else if (baseCommand == "debug") {
                cxxopts::Options options(argv[0], "Example: debug --on");

                options.allow_unrecognised_options().add_options()("on", "On")("off", "Off");

                auto result = options.parse(argc, argv);

                if (result.count("on")) {
                    printf("Debug mode now on\n");
					server->SetDebug(true);
                }
                else if (result.count("off")) {
                    printf("Debug mode now off\n");
					server->SetDebug(false);
                }
                else {
                    printf(options.help().c_str());
                }
            }
            else if (baseCommand == "connections") {
                cxxopts::Options options(argv[0], "Example: clients --list");
                options.allow_unrecognised_options().add_options()("l,list", "List clients");
                auto result = options.parse(argc, argv);

                if (result.count("l")) {
                    auto connections = server->GetConnections();
                    if (!connections.empty()) {
                        std::cout << "List of clients:\n";
                        for (const auto& connection : connections) {
                            std::cout << "- ClientID: #" << connection.GetConnectionId() << " Address: " << NetUtils::EnetAddressToString(connection.GetPeer()->address)
                                      << "\n";
                        }
                    }
                    else {
                        std::cout << "No clients are connected\n";
                    }
                }
            }
            //else if (baseCommand == "profiles") {
            //    cxxopts::Options options(argv[0], "Example: profiles --list");
            //    options.allow_unrecognised_options().add_options()("l,list", "List profiles")("g,get", "get profile")(
            //        "n,name", "get profile by name", cxxopts::value<ptl::string>())("c,cards", "get profile cards by profile name",
            //                                                                        cxxopts::value<ptl::string>());
            //    auto result = options.parse(argc, argv);

            //    if (result.count("l")) {
            //        auto &gameDataDatabase = server->.playerDB;
            //        auto &profiles = gameDataDatabase.GetProfiles();
            //        if (!profiles.empty()) {
            //            std::cout << "List of Profiles:\n";
            //            for (const auto &profile : profiles) {
            //                std::cout << "#id " << tbsg::HashString(profile->name) << " name: " << profile->name << " netID: " << profile->networkID
            //                          << " cards: " << profile->cards.size() << " decks: " << profile->decks.size() << '\n';
            //            }
            //        }
            //        else {
            //            std::cout << "No profiles could be found\n";
            //        }
            //    }
            //    else if (result.count("g")) {
            //        if (result.count("n")) {
            //            auto &gameDataDatabase = app.playerDB;
            //            auto &profiles = gameDataDatabase.GetProfiles();
            //            const auto name = result["n"].as<ptl::string>();
            //            const auto id = tbsg::HashString(name);
            //            if (profiles.has(id)) {
            //                auto &profile = profiles.at(id);
            //                std::cout << "#ID " << id << '\n'
            //                          << "Name: " << name << '\n'
            //                          << "NetID: " << profile->networkID << '\n'
            //                          << "Cards: " << profile->cards.size() << '\n'
            //                          << "Decks: " << profile->decks.size() << '\n';
            //                std::cout << "\nDeck Overview:\n";
            //                for (auto &deck : profile->decks) {
            //                    std::cout << "Name: " << deck->name << "\nCards: " << deck->cards.size() << '\n';
            //                }
            //            }
            //            else {
            //                std::cout << "Could not find profile with name " << name << "\n";
            //            }
            //        }
            //        else if (result.count("c")) {
            //            auto &gameDataDatabase = app.playerDB;
            //            auto &profiles = gameDataDatabase.GetProfiles();
            //            const auto name = result["c"].as<ptl::string>();
            //            const auto id = tbsg::HashString(name);
            //            if (profiles.has(id)) {
            //                auto &profile = profiles.at(id);
            //                std::cout << "#ID " << id << '\n'
            //                          << "Name: " << name << '\n'
            //                          << "NetID: " << profile->networkID << '\n'
            //                          << "Cards: " << profile->cards.size() << '\n'
            //                          << "Decks: " << profile->decks.size() << '\n';
            //                std::cout << "\n\nCards Overview:\n";
            //                for (auto &cards : profile->cards) {
            //                    std::cout << "Name: " << cards->meta.name << '\n';
            //                }
            //            }
            //            else {
            //                std::cout << "Could not find profile with name " << name << "\n";
            //            }
            //        }
            //        else {
            //            printf("Unknown command. Use 'help' to see available commands.\n");
            //        }
            //    }
            //    else {
            //        printf("Unknown command. Use 'help' to see available commands.\n");
            //    }
            //}
            //else if (baseCommand == "profile") {
            //    cxxopts::Options options(argv[0], "Example: profile --add --player --name Simon");
            //    options.allow_unrecognised_options().add_options()("a,add", "add profile")("p,player", "player")("b,bot", "bot")(
            //        "n,name", "get profile by name", cxxopts::value<ptl::string>());
            //    auto result = options.parse(argc, argv);

            //    if (result.count("a"))
            //    {
            //        std::cout << "Add\n";
            //        tbsg::ProfileDatabase* gameDataDatabase = nullptr;
            //        bool player = true;
            //        if (result.count("b"))
            //        {
            //            player = false;
            //            gameDataDatabase = const_cast<tbsg::ProfileDatabase*>(&app.botDB);
            //            std::cout << "bot\n";
            //        }else if(result.count("p"))
            //        {
            //            gameDataDatabase = const_cast<tbsg::ProfileDatabase*>(&app.playerDB);
            //            std::cout << "player\n";
            //        }
            //        if(result.count("n"))
            //        {
            //            const auto name = result["n"].as<ptl::string>();
            //            //gameDataDatabase->CreateProfile(name);
            //            std::cout << "Added\n";
            //        }
            //    }
            //}
   //         else if (baseCommand == "matches") {
   //             cxxopts::Options options(argv[0], "Example: matches --list");
   //             options.allow_unrecognised_options().add_options()("l,list", "List matches");
   //             auto result = options.parse(argc, argv);

   //             if (result.count("l")) {
   //                 auto &logic = app.gameLogic;
   //                 auto &gameDataDatabase = app.gameDataDatabase;
   //                 auto &matches = logic.GetMatches();
   //                 if (!matches.empty()) {
   //                     std::cout << "List of matches:\n";
   //                     for (const auto &match : matches) {
   //                         auto& player = logic.GetProfiles(match.first);
   //                         auto player1 = player.first->name;
   //                         ptl::string player2 {"No One"};
   //                         auto lobbies = server->GetLobbies();
   //                         bool playing = false;
   //                         for(auto* lobby : lobbies)
   //                         {
   //                             if(lobby->GetLobbyId() == match.first)
   //                             {
   //                                 playing = lobby->InGame();
   //                                 break;
   //                             }
   //                         }
   //                         if(player.second != nullptr)
   //                         {
   //                             player2 = player.second->name;
   //                         }
   //                         //std::cout << "LobbyID: " << match.first << " Owner:" << player1  << " Opponent: " << player2 << " playing: " << ((playing)? "yes":"no") << " ready: "<< match.second.ready <<" / "<< match.second.heros.size() << " Type: "<< ((match.second.isSinglePlayerMatch)?"Singleplayer":"Multiplayer")  << '\n';
   //                     }
   //                 }
   //                 else {
   //                     std::cout << "No matches could be found\n";
   //                 }
   //             }
   //             else {
   //                 printf("Unknown command. Use 'help' to see available commands.\n");
   //             }
   //         
   //         }
   //         else if (baseCommand == "match") 
   //         {
   //         ptl::vector<ptl::string> players;
			//ptl::vector<ptl::string> decks;
			//ptl::string seed;
			//cxxopts::Options options(argv[0], "e.g: match --simulate -p 1 -p 2 -d monsterdeck -d player1deck -d player2deck -r seed");
   //         //cxxopts::Options options(argv[0], "Example: match --simulate -c card1 -c card2"); //("c,card", "get card", cxxopts::value(cards))
   //         options.allow_unrecognised_options().add_options()("s,simulate", "simulate match")("p,playerid", "get player", cxxopts::value(players))("d,deck", "get deck", cxxopts::value(decks))("r, random", "get seed", cxxopts::value(seed));
   //         auto result = options.parse(argc, argv);
   //         if (result.count("s")) {
   //             
   //             if (result.count("p"))
   //             {
			//		auto& gamelogicRef = const_cast<tbsg::GameLogic&>(app.gameLogic);
			//		unsigned int seedconverted = 0;
			//		if (result.count("r"))
			//		{
			//			seedconverted = std::stoul(seed);
			//			cof::Info("[CMDINPUT] Setting seed to '{}'", seedconverted);
			//			gamelogicRef.SetSeed(seedconverted);
			//		}
   //                 //logic.CreateDebugMatch(cards);
			//		bool playersValid = true;
			//		if (players.size() > 1)
			//		{
			//			//TODO: CHECK IF PLAYERS EXIST IN DB, IF ONE DOESN'T, SET PLAYERSVALID TO FALSE
			//		}
			//		if (playersValid)
			//		{
			//			ptl::vector<tbsg::Deck> cardDecks{};
			//			//TODO: Replace this with getting data from DB
			//			auto& gameDataDatabase = const_cast<tbsg::GameDataModel&>(app.gameDataDatabase);
			//			for (const auto& deck : decks)
			//			{
			//				auto deckObj = gameDataDatabase.GetDeck(deck.c_str());
			//				if (deckObj != nullptr)
			//				{
			//					cardDecks.emplace_back(*deckObj);
			//				}
			//				else
			//				{
			//					cof::Warn("[CMDINPUT] Could not load deck '{}', replaced with empty deck. ", deck);
			//					cardDecks.emplace_back(tbsg::Deck{});
			//				}
			//			}
			//			ptl::vector<tbsg::Card*> monsterDeck;
			//			if (cardDecks.size() == 3)
			//			{
			//				
			//				//Create Match based on this
			//				tbsg::MatchData match = tbsg::gameplay::CreateMatchData(cardDecks, seedconverted);
			//				
			//				gamelogicRef.SimulateAIMatch(match);
			//			}
			//			else
			//			{
			//				cof::Warn("[CMDINPUT] Incorrect use of command, e.g: match --simulate -p 1 -p 2 -d monsterdeck -d player1deck -d player2deck. ");
			//			}
			//		}
   //             } else 
			//		cof::Warn("[CMDINPUT] Incorrect use of command, e.g: match --simulate -p 1 -p 2 -d monsterdeck -d player1deck -d player2deck. ");
   //             }
   //         }
            else {
				cof::Warn("[CMDINPUT] Unknown command. Use 'help' to see available commands.");
            }
        }
        else {
			cof::Warn("[CMDINPUT] Unknown command. Use 'help' to see available commands.");
        }
    }
    catch (const cxxopts::OptionException &e) { std::cout << "Error invalid option: " << e.what() << std::endl; }
}

void CMDInput::HandleParsedAndHandleCommands(tbsg::GameServer* server)
{
    std::lock_guard<std::mutex> lock_guard(m_mutex);

    auto it = m_msgQueue.begin();
    while (it != m_msgQueue.end()) {

        RTSARG args = RTSARG(*it);

        HandleCommands(args, server);

        it = m_msgQueue.erase(it);
    }
}

void CMDInput::Run()
{
    m_run.store(true);
    m_inputThread = std::thread(CMDInput::ReadCin, std::ref(m_run), std::ref(m_mutex), std::ref(m_msgQueue));
}

void CMDInput::Release()
{
    m_run.store(false);
    m_inputThread.join();
}

bool CMDInput::IsRunning() { return m_run.load(); }

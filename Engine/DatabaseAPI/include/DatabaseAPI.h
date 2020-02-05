#pragma once
#include <string>
#include "DatabaseCredentials.h"
#include <mariadb++/connection.hpp>
#include "Payloads.h"
#include "core/SparseSet.h"

namespace db
{
	class DatabaseAPI
	{
	public:
		DatabaseAPI() = default;
		explicit DatabaseAPI(DatabaseCredentials credentials, unsigned int projectId);
		~DatabaseAPI() = default;

		void OpenDatabaseConnection();

		/**
		 * \brief Get the profile id from the token the client might have provided.
		 * \return unsigned int The id of the profile. Use GetProfile to get the actual profile object. (Returns 0 when invalid)
		 */
		unsigned int GetProfileId(const ptl::string& token) const;
		/**
		 * \brief Get the profile using the associated id.
		 * \return tbsg::Profile The actual profile requested. (When id is invalid, will return a Profile object with id == 0)
		 */
		tbsg::Profile GetProfile(unsigned int profileId) const;

		ptl::sparse_set<unsigned int, tbsg::Card> GetCards() const;
		ptl::sparse_set<unsigned int, tbsg::MonsterCard> GetMonsterCards() const;
		ptl::sparse_set<unsigned int, tbsg::CardType> GetCardTypes() const;
		ptl::sparse_set<unsigned int, tbsg::CardRarity> GetCardRarities() const;

		ptl::vector<tbsg::Deck> GetDecksOfProfile(unsigned int profileId, ptl::sparse_set<unsigned int, tbsg::Card>& cards) const;
		/**
		 * \brief Get the monster deck using the id.
		 * \return tbsg::MonsterDeck The actual monster deck requested. (When id is invalid, will return a MonsterDeck object with id == 0)
		 */
		tbsg::MonsterDeck GetMonsterDeck(unsigned int id, ptl::sparse_set<unsigned int, tbsg::MonsterCard>& cards) const;
		/**
		 * \brief Get the monster deck using the unique name.
		 * \return tbsg::MonsterDeck The actual monster deck requested. (When id is invalid, will return a MonsterDeck object with id == 0)
		 */
		tbsg::MonsterDeck GetMonsterDeck(const ptl::string& name, ptl::sparse_set<unsigned int, tbsg::MonsterCard>& cards) const;

		void GetMonsterDecks(ptl::sparse_set<unsigned int, tbsg::MonsterDeck>& monsterDecks,ptl::sparse_set<unsigned int, tbsg::MonsterCard>& cards) const;

		void GetScriptsForCards(ptl::sparse_set<unsigned int, tbsg::Script>& scripts) const;
		/**
		 * \brief Get the latest revision of the script for the specified card id.
		 * \return tbsg::Script The actual script requested. (When id is invalid, will return a Script object with id == 0)
		 */
		tbsg::Script GetScriptForCard(unsigned int cardId) const;

		void GetScriptsForMonsterCards(ptl::sparse_set<unsigned int, tbsg::Script>& scripts) const;
		/**
		 * \brief Get the latest revision of the script for the specified monster card id.
		 * \return tbsg::Script The actual script requested. (When id is invalid, will return a Script object with id == 0)
		 */
		tbsg::Script GetScriptForMonsterCard(unsigned int cardId) const;
		/**
		 * \brief Get the latest revision of all the other scripts.
		 * \return ptl::vector<tbsg::Script> All the scripts.
		 */
		ptl::vector<tbsg::Script> GetOtherScripts() const;
		/**
		 * \brief Get the latest revision of the 'other' script for the specified name.
		 * \return tbsg::Script The actual script requested. (When id is invalid, will return a Script object with id == 0)
		 */
		tbsg::Script GetOtherScript(const ptl::string& scriptName) const;

		/**
		 * \brief Get all the servers that are not occupied and were online in the last 15 minutes.
		 * \warning For LobbyServer only!
		 * \return A vector of all the servers that are available for a game.
		 */
		ptl::vector<tbsg::Server> GetAvailableServers(bool production) const;
		/**
		 * \brief Register a new server in the database. Should be called by the GameServer on startup.
		 * \warning For GameServer only!
		 * \param ip The public ip address that is publicly available for clients to connect to.
		 * \param port The port the server is running on. Should be publicly available.
		 * \param hostname The hostname of the server. Servers with the same hostname as the lobby server will be preferred.
		 * \return The id of the server that was just registered.
		 */
		unsigned int RegisterServer(const ptl::string& ip, unsigned int port, ptl::string hostname, bool production) const;
		/**
		 * \brief Will update the lastOnline column for the given serverId in the servers table. Should only be used from the current GameServer.
		 * \warning For GameServer only!
		 * \param serverId The server id the GameServer received from RegisterServer(..);
		 */
		void PingServer(unsigned int serverId) const;
		/**
		 * \brief Will mark the server as occupied in the database.
		 * \param serverId The server id the GameServer received from RegisterServer(..); or GetAvailableServers();
		 * \param isOccupied Whether or not the server is occupied.
		 */
		void SetServerOccupied(unsigned int serverId, bool isOccupied) const;

		/**
		 * \brief Will log an ip the user has used.
		 * \warning Only use this definition on the LobbyServer!
		 */
		void RegisterIp(unsigned int profileId, const ptl::string& ip) const;
		/**
		 * \brief Will log an ip the user has used.
		 * \warning Only use this definition on the GameServer!
		 */
		void RegisterIp(unsigned int profileId, unsigned int serverId, const ptl::string& ip) const;

		/**
		 * \brief Register a new match in the database.
		 * \warning For LobbyServer only!
		 * \param match The match data.
		 */
		const tbsg::Match RegisterMatch(tbsg::Match match) const;
		/**
		 * \brief Mark the match as started by setting the startDate.
		 * \warning For GameServer only!
		 * \param matchId The match id received from RegisterMatch(..);
		 */
		void StartMatch(unsigned int matchId) const;
		/**
		 * \brief Mark the match as finished by setting the endDate.
		 * \warning For GameServer only!
		 * \param matchId The match id received from RegisterMatch(..);
		 */
		void EndMatch(unsigned int matchId) const;

		/**
		 * \brief Get the next match the GameServer should host. If no match is available, will return a Match object with id == 0.
		 * \Warning For GameServer only!
		 * \param serverId The current server id.
		 */
		const tbsg::Match GetNextMatchForServer(unsigned int serverId) const;

	private:
		unsigned int projectId{};

		DatabaseCredentials credentials{};

		mariadb::account_ref account{};
		mariadb::connection_ref connection{};

		ptl::string databasePrefix = "\u001b[0bm[DatabaseAPI]\u001b[0m";

	};
}

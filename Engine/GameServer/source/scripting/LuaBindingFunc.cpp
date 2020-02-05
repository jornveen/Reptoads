#include <random>
#include <utility>
#include "core/StrHash.h"
#include "scripting/LuaBindingFunc.h"
#include "gameplay/DataStructs.h"
#include "gameplay/scripting/LuaContext.h"
#include "GameDataDatabase.h"
#include "gameplay/GamePlayHelper.h"
#include "LoggingFunction.h"

void tbsg::scripting::BindMatchFunc(LuaContext& lua, MatchData* match, int& currentPlayer, ptl::vector<tbsg::MonsterCard>& monsterCards, ptl::vector<unsigned int> playedCards, GameDataDatabase* database, ResultOfRound& currentResultOfRound, ptl::vector<bool>& isHeroDead, bool& isAtBossFight, bool& isAtEndFight)
{
    lua.SetType<MatchData>("Match");
    lua.SetType<ptl::vector<tbsg::Card*>>("Hand");
    lua.SetType<Deck>("Deck");

    /**
     *
     * @brief Gets the current match.
     *
     * @return Match*
     */
    std::function<MatchData*()> GetMatch = [match]()
    {
        assert(match != nullptr);
        return match;
    };
    lua.SetFunction("GetMatch", GetMatch);

    /**
     * @brief Gets deck from array index is number in array.
     *
     * @return Deck*
     */
    std::function<ptl::vector<unsigned int>*(int)> GetDeck = [match](int index)
    {
        assert(match != nullptr);
        return &(match->playerDecks[index]);
    };
    lua.SetFunction("GetDeck", GetDeck);

    /**
     * @brief Gets hand by index.
     *
     * @param player you want to use.
     *
     * @return ptl::vector<tbsg::Card*>*
     */
    std::function<ptl::vector<unsigned int>*(int)> GetHand = [match](int playerID)
    {
        return &(match->playerHands[playerID]);
    };
    lua.SetFunction("GetHand", GetHand);


    /**
    * @brief Gets the rivalHero of the game.
    *
    * @return tbsg::Hero*
    */
    std::function<tbsg::Hero*()> GetRival = [match, &currentPlayer]()
    {
        if (currentPlayer == 0)
        {
            return &(match->heroes[1]);
        }
        return &(match->heroes[0]);
    };
    lua.SetFunction("GetRival", GetRival);

    /**
     * @brief Gets the playerHero of the game.
     *
     * @return tbsg::Hero*
     */
    std::function<tbsg::Hero*()> GetHero = [match, &currentPlayer]()
    {
        return &match->heroes[currentPlayer];
    };
    lua.SetFunction("GetHero", GetHero);


    /**
    * @brief Gets the Hero of the game by index.
    *
    * @param the ID of the hero you want to get
    *
    * @return tbsg::Hero*
    */
    std::function<tbsg::Hero*(int)> GetHeroByID = [match, &currentPlayer](int playerID)
    {
        return &match->heroes[playerID];
    };
    lua.SetFunction("GetHeroByID", GetHeroByID);

    /**
    * @brief Returns true when you are at the boss
    *
    * @return boolean
    */
    std::function<bool()> AreHeroesAtBoss = [&isAtBossFight]()
    {
        return isAtBossFight;
    };
    lua.SetFunction("AreHeroesAtBoss", AreHeroesAtBoss);

    /**
    * @brief Returns true when you are at the end fight
    *
    * @return boolean
    */
    std::function<bool()> AreHeroesAtEndFight = [&isAtEndFight]()
    {
        return isAtEndFight;
    };
    lua.SetFunction("AreHeroesAtEndFight", AreHeroesAtEndFight);

    /**
    * @brief Gets the number of turns the player is in combat with the monster. Resets if it kill it and goes to the next.
    *
    * @return int
    */
    std::function<int()> PlayerFightDuration = [match]()
    {
        return match->playerFightDuration;
    };
    lua.SetFunction("PlayerFightDuration", PlayerFightDuration);


    /**
    * @brief Sets the first player tag.
    *
    * @param player you want to Set the tag to
    *
    * @return void
    */
    std::function<void(int)> SetFirstPlayerFlag = [match](int ID)
    {
        match->nextStartingPlayer = ID;
    };
    lua.SetFunction("SetFirstPlayerFlag", SetFirstPlayerFlag);

    /**
    * @brief Switches the first player tag.
    *
    * @return void
    */
    std::function<void()> SwitcheFirstPlayerFlag = [match]()
    {
        match->nextStartingPlayer = !match->nextStartingPlayer;
    };
    lua.SetFunction("SwitcheFirstPlayerFlag", SwitcheFirstPlayerFlag);


    //Resources Hero

/**
 * @brief Gets resource of a hero.
 *
 * @param Hero you want to use.
 *
 * @return int
 */
    std::function<int(Hero*)> GetHeroResource = [](Hero* hero) {
        assert(hero != nullptr);
        return (int)hero->resource;
    };
    lua.SetFunction("GetHeroResource", GetHeroResource);

    /**
     * @brief Add resources to a hero.
     *
     * @param Hero you want to use.
     *
     * @param Amount of resource you want to add to the hero.
     *
     * @return void
     */
    std::function<void(Hero*, int)> AddHeroResource = [match, &currentResultOfRound](Hero* hero, int amount)
    {
        if (hero != nullptr)
        {
            int resource = hero->resource + amount;

            if (resource < 0)
            {
                AddChangeHero(hero, match, currentResultOfRound, hero->resource * -1, tbsg::EffectChange::Hero_Resource);

                hero->resource = 0;
            }
            else
            {
                hero->resource += amount;
                AddChangeHero(hero, match, currentResultOfRound, amount, tbsg::EffectChange::Hero_Resource);

            }
        }
    };
    lua.SetFunction("AddHeroResource", AddHeroResource);


    /**
     * @brief Sets the resources of a hero.
     *
     * @param Hero you want to use.
     *
     * @param New amount of resource.
     *
     * @return void
     */
    std::function<void(Hero*, int)> SetHeroResource = [match, &currentResultOfRound](Hero* hero, int amount)
    {
        if (hero != nullptr)
        {
            if (amount - hero->resource != 0)
            {
                AddChangeHero(hero, match, currentResultOfRound, amount - hero->resource, tbsg::EffectChange::Hero_Resource);
            }

            hero->resource = amount;
        }
    };
    lua.SetFunction("SetHeroResource", SetHeroResource);


    /**
     * @brief Steals resources from one hero and gives it to another hero.
     *
     * @param Hero you want to steal it from.
     *
     * @param Hero that gets the resources.
     *
     * @param Amount of resources you want to steal.
     *
     * @return void
     */
    std::function<void(Hero*, Hero*, int)> StealHeroResource = [match, &currentResultOfRound](Hero* target, Hero* receiver, int amount)
    {

        if (target != nullptr && receiver != nullptr)
        {
            if (target->resource >= amount)
            {
                AddChangeHero(target, match, currentResultOfRound, amount * -1, tbsg::EffectChange::Hero_Resource);
                AddChangeHero(receiver, match, currentResultOfRound, amount, tbsg::EffectChange::Hero_Resource);

                target->resource -= amount;
                receiver->resource += amount;
            }
            else
            {
                AddChangeHero(target, match, currentResultOfRound, target->resource * -1, tbsg::EffectChange::Hero_Resource);
                AddChangeHero(receiver, match, currentResultOfRound, target->resource, tbsg::EffectChange::Hero_Resource);

                receiver->resource += target->resource;
                target->resource = 0;
            }
        }
    };
    lua.SetFunction("StealHeroResource", StealHeroResource);


    //Health Hero

    /**
     * @brief Gets health of a hero.
     *
     * @param Hero you want to use.
     *
     * @return int
     */
    std::function<int(Hero*)> GetHeroHealth = [](Hero* hero)
    {
        assert(hero != nullptr);
        return (int)hero->health;
    };
    lua.SetFunction("GetHeroHealth", GetHeroHealth);


    /**
     * @brief Add health to a hero.
     *
     * @param Hero you want to use.
     *
     * @param Amount of health you want to add to the hero.
     *
     * @return void
     */
    std::function<void(Hero*, int)> AddHeroHealth = [match, &currentResultOfRound, &isHeroDead](Hero* hero, int amount)
    {
        if (hero != nullptr)
        {
            int newHealth = hero->health + amount;

            if (newHealth <= 0)
            {
                AddChangeHero(hero, match, currentResultOfRound, hero->health * -1, tbsg::EffectChange::Hero_Health);
                int heroID = AddChangeHero(hero, match, currentResultOfRound, 0, tbsg::EffectChange::Hero_Death);
                isHeroDead[heroID] = true;
                hero->health = 0;
            }
            else if (newHealth > hero->maxHealth)
            {
                AddChangeHero(hero, match, currentResultOfRound, hero->maxHealth - hero->health, tbsg::EffectChange::Hero_Health);

                hero->health = hero->maxHealth;
            }
            else
            {
                AddChangeHero(hero, match, currentResultOfRound, amount, tbsg::EffectChange::Hero_Health);
                hero->health += amount;
            }
        }
    };
    lua.SetFunction("AddHeroHealth", AddHeroHealth);


    /**
     * @brief Sets the health of a hero.
     *
     * @param Hero you want to use.
     *
     * @param New amount of health.
     *
     * @return void
     */
    std::function<void(Hero*, int)> SetHeroHealth = [match, &currentResultOfRound, &isHeroDead](Hero* hero, int amount)
    {
        if (hero != nullptr)
        {
            AddChangeHero(hero, match, currentResultOfRound, amount - hero->health, tbsg::EffectChange::Hero_Health);

            hero->health = amount;
            if (hero->health <= 0)
            {
                int heroID = AddChangeHero(hero, match, currentResultOfRound, 0, tbsg::EffectChange::Hero_Death);
                isHeroDead[heroID] = true;
                hero->health = 0;
            }

        }
    };
    lua.SetFunction("SetHeroHealth", SetHeroHealth);


    /**
     * @brief Steals health from one hero and gives it to another hero.
     *
     * @param Hero you want to steal it from.
     *
     * @param Hero that gets the health.
     *
     * @param Amount of health you want to steal.
     *
     * @return void
     */
    std::function<void(Hero*, Hero*, int)> StealHeroHealth = [match, &currentResultOfRound, &isHeroDead](Hero* target, Hero* receiver, int amount)
    {

        if (target != nullptr && receiver != nullptr)
        {
            if (target->health > amount)
            {
                AddChangeHero(target, match, currentResultOfRound, amount * -1, tbsg::EffectChange::Hero_Health);

                target->health -= amount;

                if (receiver->health + amount > receiver->maxHealth)
                {
                    AddChangeHero(receiver, match, currentResultOfRound, receiver->maxHealth - receiver->health, tbsg::EffectChange::Hero_Health);
                    receiver->health = receiver->maxHealth;
                }
                else
                {
                    AddChangeHero(receiver, match, currentResultOfRound, amount, tbsg::EffectChange::Hero_Health);
                    receiver->health += amount;
                }
            }
            else
            {
                if (receiver->health + amount > receiver->maxHealth)
                {
                    AddChangeHero(receiver, match, currentResultOfRound, receiver->maxHealth - receiver->health, tbsg::EffectChange::Hero_Health);
                    receiver->health = receiver->maxHealth;
                }
                else
                {
                    AddChangeHero(receiver, match, currentResultOfRound, target->health, tbsg::EffectChange::Hero_Health);
                    receiver->health += target->health;
                }

                AddChangeHero(target, match, currentResultOfRound, target->health * -1, tbsg::EffectChange::Hero_Health);
                int heroID = AddChangeHero(target, match, currentResultOfRound, 0, tbsg::EffectChange::Hero_Death);
                isHeroDead[heroID] = true;
                target->health = 0;
            }
        }
    };
    lua.SetFunction("StealHeroHealth", StealHeroHealth);


    //Armor Hero

    /**
     * @brief Gets armor of a hero.
     *
     * @param Hero you want to use.
     *
     * @return int
     */
    std::function<int(Hero*)> GetHeroArmor = [](Hero* hero)
    {
        assert(hero != nullptr);
        return hero->armor;
    };
    lua.SetFunction("GetHeroArmor", GetHeroArmor);


    /**
     * @brief Add armor to a hero.
     *
     * @param Hero you want to use.
     *
     * @param Amount of armor you want to add to the hero.
     *
     * @return void
     */
    std::function<void(Hero*, int)> AddHeroArmor = [match, &currentResultOfRound](Hero* hero, int amount)
    {
        if (hero != nullptr)
        {
            int armor = hero->armor + amount;

            if (armor < 0)
            {
                AddChangeHero(hero, match, currentResultOfRound, hero->armor * -1, tbsg::EffectChange::Hero_Armor);

                hero->armor = 0;
            }
            else
            {
                AddChangeHero(hero, match, currentResultOfRound, amount, tbsg::EffectChange::Hero_Armor);

                hero->armor += amount;
            }
        }
    };
    lua.SetFunction("AddHeroArmor", AddHeroArmor);


    /**
     * @brief Sets the armor of a hero.
     *
     * @param Hero you want to use.
     *
     * @param New amount of armor.
     *
     * @return void
     */
    std::function<void(Hero*, int)> SetHeroArmor = [match, &currentResultOfRound](Hero* hero, int amount)
    {
        if (hero != nullptr)
        {
            int newArmor = amount - hero->armor;
            if (newArmor < 0)
            {
                AddChangeHero(hero, match, currentResultOfRound, 0 - hero->armor, tbsg::EffectChange::Hero_Armor);
                hero->armor = 0;
            }
            else
            {
                AddChangeHero(hero, match, currentResultOfRound, amount - hero->armor, tbsg::EffectChange::Hero_Armor);
                hero->armor = amount;
            }
        }
    };
    lua.SetFunction("SetHeroArmor", SetHeroArmor);


    /**
     * @brief Steals armor from one hero and gives it to another hero.
     *
     * @param Hero you want to steal it from.
     *
     * @param Hero that gets the armor.
     *
     * @param Amount of armor you want to steal.
     *
     * @return void
     */
    std::function<void(Hero*, Hero*, int)> StealHeroArmor = [match, &currentResultOfRound](Hero* target, Hero* receiver, int amount)
    {
        if (target != nullptr && receiver != nullptr)
        {
            if (target->armor >= amount)
            {
                AddChangeHero(target, match, currentResultOfRound, amount * -1, tbsg::EffectChange::Hero_Armor);
                AddChangeHero(receiver, match, currentResultOfRound, amount, tbsg::EffectChange::Hero_Armor);

                target->armor -= amount;
                receiver->armor += amount;
            }
            else
            {
                AddChangeHero(target, match, currentResultOfRound, target->armor * -1, tbsg::EffectChange::Hero_Armor);
                AddChangeHero(receiver, match, currentResultOfRound, target->armor, tbsg::EffectChange::Hero_Armor);

                receiver->armor += target->armor;
                target->armor = 0;
            }
        }
    };
    lua.SetFunction("StealHeroArmor", StealHeroArmor);

    //Attack Hero

    /**
     * @brief Gets attack of a hero.
     *
     * @param Hero you want to use.
     *
     * @return int
     */
    std::function<int(Hero*)> GetHeroAttack = [](Hero* hero)
    {
        assert(hero != nullptr);
        return hero->attack;
    };
    lua.SetFunction("GetHeroAttack", GetHeroAttack);


    /**
     * @brief Add attack to a hero.
     *
     * @param Hero you want to use.
     *
     * @param Amount of attack you want to add to the hero.
     *
     * @return void
     */
    std::function<void(Hero*, int)> AddHeroAttack = [match, &currentResultOfRound](Hero* hero, int amount)
    {
        if (hero != nullptr)
        {
            int attack = hero->attack + amount;

            if (attack < 0)
            {
                AddChangeHero(hero, match, currentResultOfRound, hero->attack * -1, tbsg::EffectChange::Hero_Attack);

                hero->attack = 0;
            }
            else
            {
                AddChangeHero(hero, match, currentResultOfRound, amount, tbsg::EffectChange::Hero_Attack);

                hero->attack += amount;
            }
        }
    };
    lua.SetFunction("AddHeroAttack", AddHeroAttack);


    /**
     * @brief Sets the attack of a hero.
     *
     * @param Hero you want to use.
     *
     * @param New amount of attack.
     *
     * @return void
     */
    std::function<void(Hero*, int)> SetHeroAttack = [match, &currentResultOfRound](Hero* hero, int amount)
    {
        if (hero != nullptr)
        {
            if (amount - hero->attack != 0)
                AddChangeHero(hero, match, currentResultOfRound, amount - hero->attack, tbsg::EffectChange::Hero_Attack);

            hero->attack = amount;
        }
    };
    lua.SetFunction("SetHeroAttack", SetHeroAttack);


    /**
     * @brief Steals attack from one hero and gives it to another hero.
     *
     * @param Hero you want to steal it from.
     *
     * @param Hero that gets the attack.
     *
     * @param Amount of attack you want to steal.
     *
     * @return void
     */
    std::function<void(Hero*, Hero*, int)> StealHeroAttack = [match, &currentResultOfRound](Hero* target, Hero* receiver, int amount)
    {

        if (target != nullptr && receiver != nullptr)
        {
            if (target->attack >= amount)
            {
                AddChangeHero(target, match, currentResultOfRound, amount * -1, tbsg::EffectChange::Hero_Attack);
                AddChangeHero(receiver, match, currentResultOfRound, amount, tbsg::EffectChange::Hero_Attack);

                target->attack -= amount;
                receiver->attack += amount;
            }
            else
            {
                AddChangeHero(target, match, currentResultOfRound, target->attack * -1, tbsg::EffectChange::Hero_Attack);
                AddChangeHero(receiver, match, currentResultOfRound, target->attack, tbsg::EffectChange::Hero_Attack);

                receiver->attack += target->attack;
                target->attack = 0;
            }
        }
    };
    lua.SetFunction("StealHeroAttack", StealHeroAttack);

    /**
     * @brief Gets the total attack of a hero.
     *
     * @param Hero you want to use.
     *
     * @return int
     */
    std::function<int(Hero*)> GetHeroTotalAttack = [](Hero* hero)
    {
        assert(hero != nullptr);
        int totalAttack = hero->attack;

        if (hero->weapon != nullptr)
        {
            totalAttack += hero->weapon->attack;
        }

        return totalAttack;
    };
    lua.SetFunction("GetHeroTotalAttack", GetHeroTotalAttack);

    //Weapon Hero
    /**
     * @brief Gets weapon of a hero.
     *
     * @param Hero you want to use.
     *
     * @return Weapon*
     */
    std::function<Weapon*(Hero*)> GetHeroWeapon = [](Hero* hero)
    {
        assert(hero != nullptr);
        return hero->weapon;
    };
    lua.SetFunction("GetHeroWeapon", GetHeroWeapon);

    /**
     * @brief Sets the weapon of a hero.
     *
     * @param Hero you want to use.
     *
     * @param New weapon of hero.
     *
     * @return void
     */
    std::function<void(Hero*, Weapon*)> SetHeroWeapon = [match, &currentResultOfRound](Hero* hero, Weapon* weapon)
    {
        if (hero != nullptr)
        {
            if (weapon != nullptr)
            {
                AddChangeHero(hero, match, currentResultOfRound, weapon->attack, tbsg::EffectChange::Hero_Weapon_Attack);
                AddChangeHero(hero, match, currentResultOfRound, weapon->durability, tbsg::EffectChange::Hero_Weapon_Durability);
            }
            else if (hero->weapon != nullptr)
            {
                AddChangeHero(hero, match, currentResultOfRound, 0 - hero->weapon->attack, tbsg::EffectChange::Hero_Weapon_Attack);
                AddChangeHero(hero, match, currentResultOfRound, 0 - hero->weapon->durability, tbsg::EffectChange::Hero_Weapon_Durability);
            }

            hero->weapon = weapon;
        }
    };
    lua.SetFunction("SetHeroWeapon", SetHeroWeapon);


    /**
     * @brief Steals the weapon from one hero and gives it to another hero.
     *
     * @param Hero you want to steal it from.
     *
     * @param Hero that gets the weapon.
     *
     * @return void
     */
    std::function<void(Hero*, Hero*)> StealHeroWeapon = [match, &currentResultOfRound](Hero* target, Hero* receiver)
    {
        if (target != nullptr && receiver != nullptr)
        {
            if (target->weapon != nullptr)
            {
                AddChangeHero(target, match, currentResultOfRound, target->weapon->attack * -1, tbsg::EffectChange::Hero_Weapon_Attack);
                AddChangeHero(target, match, currentResultOfRound, target->weapon->durability * -1, tbsg::EffectChange::Hero_Weapon_Durability);

                AddChangeHero(receiver, match, currentResultOfRound, target->weapon->attack, tbsg::EffectChange::Hero_Weapon_Attack);
                AddChangeHero(receiver, match, currentResultOfRound, target->weapon->durability, tbsg::EffectChange::Hero_Weapon_Durability);

                receiver->weapon = target->weapon;
                target->weapon = nullptr;
            }
        }
    };
    lua.SetFunction("StealHeroWeapon", StealHeroWeapon);

    /**
     * @brief Creates a weapon and returns it.
     *
     * @param Attack of the weapon.
     *
     * @param durability of the weapon.
     *
     * @return Weapon*
     */
    std::function<Weapon*(int, int)> CreateHeroWeapon = [](int weaponAttack, int weaponDurability)
    {
        Weapon* weapon = new Weapon();
        weapon->attack = weaponAttack;
        weapon->durability = weaponDurability;
        return weapon;
    };
    lua.SetFunction("CreateHeroWeapon", CreateHeroWeapon);



    //Weapon Hero Attack
    /**
     * @brief Gets attack of a weapon.
     *
     * @param Hero you want to use.
     *
     * @return int
     */
    std::function<int(Hero*)> GetWeaponAttack = [](Hero* hero)
    {
        if (hero != nullptr && hero->weapon != nullptr)
        {
            return (int)hero->weapon->attack;
        }
        else
        {
            return 0;
        }
    };
    lua.SetFunction("GetWeaponAttack", GetWeaponAttack);

    /**
     * @brief Add attack to a weapon.
     *
     * @param Hero you want to use.
     *
     * @param Amount of attack you want to add to the weapon.
     *
     * @return void
     */
    std::function<void(Hero*, int)> AddWeaponAttack = [match, &currentResultOfRound](Hero* hero, int amount)
    {
        if (hero != nullptr && hero->weapon != nullptr)
        {
            int weaponAttack = hero->weapon->attack + amount;

            if (weaponAttack < 0)
            {
                AddChangeHero(hero, match, currentResultOfRound, hero->weapon->attack * -1, tbsg::EffectChange::Hero_Weapon_Attack);

                hero->weapon->attack = 0;
            }
            else
            {
                AddChangeHero(hero, match, currentResultOfRound, amount, tbsg::EffectChange::Hero_Weapon_Attack);

                hero->weapon->attack += amount;
            }
        }
    };
    lua.SetFunction("AddWeaponAttack", AddWeaponAttack);


    /**
     * @brief Sets the attack of a weapon.
     *
     * @param Hero you want to use.
     *
     * @param New amount of attack.
     *
     * @return void
     */
    std::function<void(Hero*, int)> SetWeaponAttack = [match, &currentResultOfRound](Hero* hero, int amount)
    {
        if (hero != nullptr && hero->weapon != nullptr)
        {
            AddChangeHero(hero, match, currentResultOfRound, amount - hero->weapon->attack, tbsg::EffectChange::Hero_Weapon_Attack);

            hero->weapon->attack = amount;
        }
    };
    lua.SetFunction("SetWeaponAttack", SetWeaponAttack);

    //Weapon Hero Durability

    /**
     * @brief Gets durability of a weapon.
     *
     * @param Hero you want to use.
     *
     * @return int
     */
    std::function<int(Hero*)> GetWeaponDurability = [](Hero* hero)
    {
        if (hero != nullptr && hero->weapon != nullptr)
        {
            return (int)hero->weapon->durability;
        }
        else
        {
            return 0;
        }
    };
    lua.SetFunction("GetWeaponDurability", GetWeaponDurability);


    /**
     * @brief Add durability to a weapon.
     *
     * @param Hero you want to use.
     *
     * @param Amount of durability you want to add to the weapon.
     *
     * @return void
     */
    std::function<void(Hero*, int)> AddWeaponDurability = [match, &currentResultOfRound](Hero* hero, int amount)
    {
        if (hero != nullptr && hero->weapon != nullptr)
        {
            int weaponDurability = hero->weapon->durability + amount;

            if (weaponDurability < 0)
            {
                AddChangeHero(hero, match, currentResultOfRound, hero->weapon->durability * -1, tbsg::EffectChange::Hero_Weapon_Durability);
                AddChangeHero(hero, match, currentResultOfRound, 0 - hero->weapon->attack, tbsg::EffectChange::Hero_Weapon_Attack);

                hero->weapon->durability = 0;
                hero->weapon = nullptr;
            }
            else
            {
                AddChangeHero(hero, match, currentResultOfRound, amount, tbsg::EffectChange::Hero_Weapon_Durability);

                hero->weapon->durability += amount;
            }
        }
    };
    lua.SetFunction("AddWeaponDurability", AddWeaponDurability);


    /**
     * @brief Sets the durability of a weapon.
     *
     * @param Hero you want to use.
     *
     * @param New amount of durability.
     *
     * @return void
     */
    std::function<void(Hero*, int)> SetWeaponDurability = [match, &currentResultOfRound](Hero* hero, int amount)
    {
        if (hero != nullptr && hero->weapon != nullptr)
        {
            AddChangeHero(hero, match, currentResultOfRound, amount - hero->weapon->durability, tbsg::EffectChange::Hero_Weapon_Durability);
            if (amount == 0)
            {
                AddChangeHero(hero, match, currentResultOfRound, 0 - hero->weapon->attack, tbsg::EffectChange::Hero_Weapon_Attack);
                hero->weapon = nullptr;
            }
            else
            {
                hero->weapon->durability = amount;
            }
        }
    };
    lua.SetFunction("SetWeaponDurability", SetWeaponDurability);


    /**
    * @brief Gets the monster you are currently facing
    *
    * @return tbsg::Hero*
    */
    std::function<tbsg::MonsterCard*()> GetCurrentMonster = [match, &currentPlayer]()
    {
        if (match->monsterCards.empty())
        {
            MonsterCard* monsterCard = nullptr;
            return monsterCard;
        }
        return &match->monsterCards[0];
    };
    lua.SetFunction("GetCurrentMonster", GetCurrentMonster);


    /**
    * @brief Gets the played card by player and index.
    *
    * @param player you want to get the played cards from.
    *
    * @param number of the card you want to check.
    *
    * @return Card*
    */
    std::function<Card*(int)> GetPlayedCard = [playedCards, database](int playerID)
    {
        return database->GetCard(playedCards[playerID]);
    };
    lua.SetFunction("GetPlayedCard", GetPlayedCard);


    /**
    * @brief Gets a card from hand.
    *
    * @param player you want to use.
    *
    * @param index of the card you want to get.
    *
    * @return Card*
    */
    std::function<const Card*(int, int)> GetCardFromHand = [match, database](int playerID, int index)
    {
        const Card* cCard = database->GetCard(match->playerHands[playerID][index]);
        return cCard;
    };
    lua.SetFunction("GetCardFromHand", GetCardFromHand);

    /**
    * @brief Gets the amount of cards you have in your hand.
    *
    * @param player you want to use.
    *
    * @return int
    */
    std::function<int(int)> GetHandSize = [&match](int playerID)
    {
        return match->playerHands[playerID].size();
    };
    lua.SetFunction("GetHandSize", GetHandSize);

    /**
    * @brief Gets the amount of cards you have in your hand.
    *
    * @param player you want to use.
    *
    * @return int
    */
    std::function<int(int)> GetDeckSize = [&match](int playerID)
    {
        return match->playerDecks[playerID].size();
    };
    lua.SetFunction("GetDeckSize", GetDeckSize);

    /**
    * @brief Gets the amount of cards you have in your hand.
    *
    * @param player you want to use.
    *
    * @return int
    */
    std::function<int(int)> GetDiscardSize = [&match](int playerID)
    {
        return match->playerDiscards[playerID].size();
    };
    lua.SetFunction("GetDiscardSize", GetDiscardSize);

    /**
     * @brief Gets the cardIndex of a card in your hand by name.
     *
     * @param player you want to use.
     *
     * @param name of the card.
     *
     * @return int
     */
    std::function<int(int, ptl::string)> GetCardInHandByName = [match, database](int playerID, ptl::string cardName)
    {
        for (size_t i = 0; i < match->playerHands[playerID].size(); ++i)
        {
            if (database->GetCard(match->playerHands[playerID][i])->meta.name == cardName)
            {
                return static_cast<int>(i);
            }
        }
        return -1;
    };
    lua.SetFunction("GetCardInHandByName", GetCardInHandByName);

    /**
     * @brief Gets the amount of cards you have in your hand by name.
     *
     * @param player you want to use.
     *
     * @param name of the card.
     *
     * @return int
     */
    std::function<int(int, ptl::string)> GetAmountOfCardsInHandByName = [match, database](int playerID, ptl::string cardName)
    {
        int index = 0;
        for (size_t i = 0; i < match->playerHands[playerID].size(); ++i)
        {
            if (database->GetCard(match->playerHands[playerID][i])->meta.name == cardName)
            {
                index++;
            }
        }
        return index;
    };
    lua.SetFunction("GetAmountOfCardsInHandByName", GetAmountOfCardsInHandByName);



    /**
     * @brief Gets the current playerID.
     *
     * @return int
     */
    std::function<int()> GetCurrentplayerID = [&currentPlayer]()
    {
        return currentPlayer;
    };
    lua.SetFunction("GetCurrentPlayerID", GetCurrentplayerID);

    /**
     * @brief Gets the current rivalID.
     *
     * @return int
     */
    std::function<int()> GetCurrentRivalID = [&currentPlayer]()
    {
        if (currentPlayer == 0)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    };
    lua.SetFunction("GetCurrentRivalID", GetCurrentRivalID);

    /**
    * @brief Adds a card with a specific card type to the players deck.
    *
    * @param player you want to use.
    *
    * @param Type of the card.
    *
    * @return void
    */
    std::function<void(int, ptl::string)> AddCardToDeckByType = [database, match, &currentResultOfRound](int playerID, ptl::string cardType)
    {
		ptl::vector<unsigned int > cards = database->GetCardsByType(std::move(cardType));

        if (!cards.empty())
        {
	        const unsigned randomIndex = tbsg::gameplay::GetRandomIntInRange(0, cards.size() - 1, match->gameSeed);

            if (cards.size() > randomIndex)
            {
                match->playerDecks[playerID].push_back(cards[randomIndex]);
                gameplay::ShuffleCards(match->playerDecks[playerID]);
                tbsg::Change change{ tbsg::EffectChange::Deck_AddCard, static_cast<int>(cards[randomIndex]), static_cast<unsigned int>(playerID) };
                currentResultOfRound.results.push_back(change);
				cof::Info("[AddCardToDeckByType] added {} to players deck", change.change);
            }

        }else
        {
			cof::Error("[AddCardToDeckByType] could not add card by type because could not find any");
        }
    };
    lua.SetFunction("AddCardToDeckByType", AddCardToDeckByType);

    /**
    * @brief Adds a specific card to the players deck by name.
    *
    * @param player you want to use.
    *
    * @param Name of the card.
    *
    * @return void
    */
    std::function<void(int, ptl::string)> AddCardToDeckByName = [database, match, &currentResultOfRound](int playerID, ptl::string cardName)
    {
        Card* card = database->GetCard(ptl::string(cardName));

        if (card != nullptr)
        {
            match->playerDecks[playerID].push_back(card->id);
            gameplay::ShuffleCards(match->playerDecks[playerID]);
            tbsg::Change change{ tbsg::EffectChange::Deck_AddCard, static_cast<int>(card->id), static_cast<unsigned int>(playerID) };
            currentResultOfRound.results.push_back(change);
			cof::Info("[AddCardToDeckByName] added {} to deck of player {}", cardName, playerID);
        }else
        {
			cof::Error("[AddCardToDeckByName] could not added {} to deck of player {}", cardName, playerID);
        }
    };
    lua.SetFunction("AddCardToDeckByName", AddCardToDeckByName);


    /**
    * @brief Adds a specific card to the players deck by name.
    *
    * @param player you want to use.
    *
    * @param Name of the card.
    *
    * @return void
    */
    std::function<void(int, ptl::string)> AddCardToDiscardByName = [database, match, &currentResultOfRound](int playerID, ptl::string cardName)
    {
        Card* card = database->GetCard(cardName);

        if (card != nullptr)
        {
			auto checkIfinHand = std::find_if(match->playerHands[playerID].begin(), match->playerHands[playerID].end(),[cardId = card->id](const auto id)
			{
				return id == cardId;
			});
			if(checkIfinHand != match->playerHands[playerID].end()){
            match->playerDiscards[playerID].push_back(card->id);
            tbsg::Change change{ tbsg::EffectChange::Discard_AddCard, static_cast<int>(card->id), static_cast<unsigned int>(playerID) };
            currentResultOfRound.results.push_back(change);
			}else
			{
				cof::Warn("[AddCardToDiscardByName] could not remove card from hand which was not there! Therefore we do nothing.");
			}
        }
    };
    lua.SetFunction("AddCardToDiscardByName", AddCardToDiscardByName);

    /**
     * @brief Adds a card with a specific card type to the player hand.
     *
     * @param player you want to use.
     *
     * @param Type of the card.
     *
     * @return void
     */ 
     std::function<void(int, ptl::string)> AddCardToHandByType = [database, match, &currentResultOfRound](int playerID, ptl::string cardType)
     {
		 auto cards = std::move(database->GetCardsByType(cardType));
     
         if (!cards.empty())
         {
             int randomIndex = tbsg::gameplay::GetRandomIntInRange(0, cards.size() - 1, match->gameSeed);
             
             if (cards.size() > randomIndex)
             {
				 auto card = cards[randomIndex];
                 match->playerHands[playerID].push_back(card);

                 tbsg::Change change{ tbsg::EffectChange::Hand_AddCard, static_cast<int>(cards[randomIndex]), static_cast<unsigned int>(playerID) };
                 currentResultOfRound.results.push_back(change);
             }

         }
     };
     lua.SetFunction("AddCardToHandByType", AddCardToHandByType);

     /**
     * @brief Adds a specific card to the players hand by name.
     *
     * @param player you want to use.
     *
     * @param Name of the card.
     *
     * @return void
     */
     std::function<void(int, ptl::string)> AddCardToHandByName = [database, match, &currentResultOfRound](int playerID, ptl::string cardName)
     {
         Card* card = database->GetCard(ptl::string(cardName));

         if (card != nullptr)
         {
            match->playerHands[playerID].push_back(card->id);

            tbsg::Change change{ tbsg::EffectChange::Hand_AddCard, static_cast<int>(card->id), static_cast<unsigned int>(playerID) };
            currentResultOfRound.results.push_back(change);
         }
     };
     lua.SetFunction("AddCardToHandByName", AddCardToHandByName);

    /**
     * @brief Adds a card to hand by player.
     *
     * @param player you want to use.
     *
     * @param name of the card.
     *
     * @return void
     */
    //TODO: We cannot get a card by its name, only by ID and SimpleHash() is not supported anymore
    //std::function<void(int, ptl::string)> AddCardToHand = [database, match, &currentResultOfRound](int playerID, ptl::string cardName)
    //{
    //    const unsigned int cardHash = tbsg::SimpleHash(cardName.c_str());
    //    const Card* constCard = database->GetCard(cardHash);
    //
    //    if (constCard != nullptr) {
    //        match->playerHands[playerID].push_back(cardHash);
    //
    //        tbsg::Change change{ tbsg::EffectChange::Hand_AddCard, cardHash, playerID};
    //        currentResultOfRound.results.push_back(change);
    //    }
    //};
    //lua.SetFunction("AddCardToHand", AddCardToHand);

    /**
     * @brief Adds an amount of cards to hand.
     *
     * @param player you want to use.
     *
     * @param name of the card.
     *
     * @param amount of cards you want to add
     *
     * @return void
     */
    // TODO: We cannot get a card by its name, only by ID and SimpleHash() is not supported anymore
    //std::function<void(int, ptl::string, int)> AddCardsToHand = [database, &match, &currentResultOfRound](int playerID, ptl::string cardName, int amount)
    //{
    //    const unsigned int cardHash = tbsg::SimpleHash(cardName.c_str());
    //    const Card* constCard = database->GetCard(cardHash);
    //
    //    if (constCard != nullptr) {
    //        for (int i = 0; i < amount; ++i)
    //        {
    //            match->playerHands[playerID].push_back(cardHash);
    //
    //            tbsg::Change change{ tbsg::EffectChange::Hand_AddCard, cardHash, playerID};
    //            currentResultOfRound.results.push_back(change);
    //        }
    //    }
    //};
    //lua.SetFunction("AddCardsToHand", AddCardsToHand);


    /**
     * @brief Removes a card from hand by index.
     *
     * @param player you want to use.
     *
     * @param index of the card you want to remove.
     *
     * @return void
     */
    std::function<void(int, int)> RemoveCardFromHand = [match, &currentResultOfRound, database](int playerID, int index) 
    {
        assert(match != nullptr);
        if (match->playerHands[playerID].size() > index)
        {

            tbsg::Change change{ tbsg::EffectChange::Hand_RemoveCard, static_cast<int>(match->playerHands[playerID][index]), static_cast<unsigned int>(playerID)};
            currentResultOfRound.results.push_back(change);

            match->playerHands[playerID].erase(match->playerHands[playerID].begin() + index);
        }
    };
    lua.SetFunction("RemoveCardFromHand", RemoveCardFromHand);

    /**
    * @brief Removes all card from a player.
    *
    * @param player you want to use.
    *
    * @return void
    */
    std::function<void(int)> RemoveAllCardsFromHand = [match, &currentResultOfRound, database](int playerID) 
    {
        assert(match != nullptr);
        for (int i = 0; i < match->playerHands[playerID].size(); ++i)
        {
            tbsg::Change change{ tbsg::EffectChange::Hand_RemoveCard, static_cast<int>(match->playerHands[playerID][i]), static_cast<unsigned int>(playerID) };
            currentResultOfRound.results.push_back(change);

           
        }
		match->playerHands[playerID].clear();
    };
    lua.SetFunction("RemoveAllCardsFromHand", RemoveAllCardsFromHand);

    /**
     * @brief Removes a card from hand by name.
     *
     * @param player you want to use.
     *
     * @param name of the card.
     *
     * @return void
     */
    std::function<void(int, ptl::string)> RemoveCardFromHandByName = [match, &currentResultOfRound, database](int playerID, ptl::string cardName) 
    {
        assert(match != nullptr);

        for (size_t i = 0; i < match->playerHands[playerID].size(); ++i)
        {
            if (database->GetCard(match->playerHands[playerID][i])->meta.name == cardName)
            {
                tbsg::Change change{ tbsg::EffectChange::Hand_RemoveCard, static_cast<int>(match->playerHands[playerID][i]), static_cast<unsigned int>(playerID)};
                currentResultOfRound.results.push_back(change);

                match->playerHands[playerID].erase(match->playerHands[playerID].begin() + i);
                break;
            }
        }
    };
    lua.SetFunction("RemoveCardFromHandByName", RemoveCardFromHandByName);

    /**
    * @brief Removes all card from hand by name.
    *
    * @param player you want to use.
    *
    * @param name of the card.
    *
    * @return void
    */
    std::function<void(int, ptl::string, int)> RemoveCardsFromHandByName = [&match, &currentResultOfRound, database](int playerID, ptl::string cardName, int amount) 
    {
        assert(match != nullptr);
        int index = 0;
        for (size_t i = 0; i < match->playerHands[playerID].size(); ++i)
        {
            if (database->GetCard(match->playerHands[playerID][i])->meta.name == cardName)
            {
                tbsg::Change change{ tbsg::EffectChange::Hand_RemoveCard, static_cast<int>(match->playerHands[playerID][i]), static_cast<unsigned int>(playerID)};
                currentResultOfRound.results.push_back(change);

                match->playerHands[playerID].erase(match->playerHands[playerID].begin() + i);
                index++;

                if (index == amount)
                    break;
            }
        }
    };
    lua.SetFunction("RemoveCardsFromHandByName", RemoveCardsFromHandByName);

    /**
    * @brief Removes a random card from hand by name.
    *
    * @param player you want to use.
    *
    * @param name of the card.
    *
    * @return void
    */
    std::function<void(int, ptl::string)> RemoveRandomCardFromHandByName = [match, &currentResultOfRound, database](int playerID, ptl::string cardName)
    {
        assert(match != nullptr);
        ptl::vector<int> cardIndexs;
        for (auto i = 0; i < match->playerHands[playerID].size(); ++i)
        {
            if (database->GetCard(match->playerHands[playerID][i])->meta.name == cardName)
            {
                cardIndexs.push_back(i);
            }
        }

        if (cardIndexs.size() > 0)
        {
            auto randomIndex = tbsg::gameplay::GetRandomIntInRange(0, cardIndexs.size(), match->gameSeed);

            tbsg::Change change{ tbsg::EffectChange::Hand_RemoveCard, static_cast<int>(match->playerHands[playerID][cardIndexs[randomIndex]]), static_cast<unsigned int>(playerID)};
            currentResultOfRound.results.push_back(change);

            match->playerHands[playerID].erase(match->playerHands[playerID].begin() + cardIndexs[randomIndex]);
        }
    };
    lua.SetFunction("RemoveRandomCardFromHandByName", RemoveRandomCardFromHandByName);

    /**
     * @brief Gets a card from deck by index.
     *
     * @param player you want to use.
     *
     * @param index of the card you want to get.
     *
     * @return Card*
     */
    std::function<Card*(int, int)> GetDeckCard = [match, database](int playerID, int index) 
    {
        assert(match != nullptr);
        Card* card = database->GetCard(match->playerDecks[playerID][index]);
        return card;
    };
    lua.SetFunction("GetDeckCard", GetDeckCard);

    /**
     * @brief Gets a card from deck by name
     *
     * @param player you want to use.
     *
     * @param name of the card.
     *
     * @return int
     */
    std::function<Card*(int, ptl::string)> GetCardInDeckByName = [match, database](int playerID, ptl::string cardName)
    {
        assert(match != nullptr);
        for (size_t i = 0; i < match->playerDecks[playerID].size(); ++i)
        {
            Card* card = database->GetCard(match->playerDecks[playerID][i]);
            if (card->meta.name == cardName)
            {
                return card;
            }
        }
        Card* emptyCard = nullptr;
        return emptyCard;
    };
    lua.SetFunction("GetCardInDeckByName", GetCardInDeckByName);

    /**
     * @brief Adds a card to the deck
     *
     * @param player you want to use.
     *
     * @param name of the card.
     *
     * @return void
     */
    // TODO: We cannot get a card by its name, only by ID and SimpleHash() is not supported anymore
    //std::function<void(int, ptl::string)> AddCardToDeck = [match, database, &currentResultOfRound](int playerID, ptl::string cardName) 
    //{
    //    assert(match != nullptr);
    //    unsigned int cardHash = tbsg::SimpleHash(cardName.c_str());
    //    const Card* constCard = database->GetCard(cardHash);
    //    if (constCard != nullptr)
    //    {
    //        Card* card = new Card(*constCard);
    //        match->playerDecks[playerID].push_back(cardHash);
    //
    //        tbsg::Change change{ tbsg::EffectChange::Deck_AddCard, cardHash, playerID};
    //        currentResultOfRound.results.push_back(change);
    //
    //    }
    //};
    //lua.SetFunction("AddCardToDeck", AddCardToDeck);

    /**
     * @brief Adds a card to the deck by name.
     *
     * @param player you want to use.
     *
     * @param name of the card.
     *
     * @param amount of cards you want to add.
     *
     * @return void
     */
    // TODO: We cannot get a card by its name, only by ID and SimpleHash() is not supported anymore
    //std::function<void(int, ptl::string, int)> AddCardsToDeck = [match, database, &currentResultOfRound](int playerID, ptl::string cardName, int amount) 
    //{
    //    assert(match != nullptr);
    //    unsigned int cardHash = tbsg::SimpleHash(cardName.c_str());
    //    const Card* constCard = database->GetCard(cardHash);
    //    if (constCard != nullptr)
    //    {
    //        for (int i = 0; i < amount; ++i)
    //        {
    //            Card* card = new Card(*constCard);
    //            match->playerDecks[playerID].push_back(cardHash);
    //
    //            tbsg::Change change{ tbsg::EffectChange::Deck_AddCard, tbsg::SimpleHash(cardName.c_str()), playerID };
    //            currentResultOfRound.results.push_back(change);
    //        }
    //    }
    //};
    //lua.SetFunction("AddCardsToDeck", AddCardsToDeck);

    /**
    * @brief Removes random card from deck.
    *
    * @param player you want to use.
    *
    * @return void
    */
    std::function<void(int)> RemoveRandomCardFromDeck = [match, &currentResultOfRound](int playerID) 
    {
	
        assert(match != nullptr);

        int randomIndex = tbsg::gameplay::GetRandomIntInRange(0, match->playerDecks[playerID].size() - 1, match->gameSeed);

        tbsg::Change change{ tbsg::EffectChange::Deck_RemoveCard, static_cast<int>(match->playerDecks[playerID][randomIndex]), static_cast<unsigned int>(playerID)};
        currentResultOfRound.results.push_back(change);

        match->playerDecks[playerID].erase(match->playerDecks[playerID].begin() + randomIndex);

    };
    lua.SetFunction("RemoveRandomCardFromDeck", RemoveRandomCardFromDeck);

    /**
    * @brief Remove random cards from deck.
    *
    * @param player you want to use.
    *
    * @param amount of cards you want to remove.
    *
    * @return void
    */
    std::function<void(int, int)> RemoveRandomCardsFromDeck = [match, &currentResultOfRound](int playerID, int amount) 
    {

        assert(match != nullptr);

        for (int i = 0; i < amount; ++i)
        {
            int randomIndex = tbsg::gameplay::GetRandomIntInRange(0, match->playerDecks[playerID].size() - 1, match->gameSeed);;

            tbsg::Change change{tbsg::EffectChange::Deck_RemoveCard, static_cast<int>(match->playerDecks[playerID][randomIndex]), static_cast<unsigned int>(playerID) };
            currentResultOfRound.results.push_back(change);

            match->playerDecks[playerID].erase(match->playerDecks[playerID].begin() + randomIndex);
        }
    };
    lua.SetFunction("RemoveRandomCardsFromDeck", RemoveRandomCardsFromDeck);


    /**
    * @brief Remove random card from hand.
    *
    * @param player you want to use.
    *
    * @return void
    */
    std::function<void(int)> RemoveRandomCardFromHand = [match, &currentResultOfRound, database](int playerID) 
    {

        assert(match != nullptr);
        ptl::vector<Card*> hand;
        for (int i = 0; i < match->playerHands[playerID].size(); ++i)
        {
            hand.push_back(database->GetCard(match->playerHands[playerID][i]));
        }
        if (hand.size() > 0)
        {
            int randomIndex = tbsg::gameplay::GetRandomIntInRange(0, hand.size() - 1, match->gameSeed);

            tbsg::Change change{ tbsg::EffectChange::Hand_RemoveCard, static_cast<int>(match->playerHands[playerID][randomIndex]), static_cast<unsigned int>(playerID) };
            currentResultOfRound.results.push_back(change);

            match->playerHands[playerID].erase(match->playerHands[playerID].begin() + randomIndex);
        }
    };
    lua.SetFunction("RemoveRandomCardFromHand", RemoveRandomCardFromHand);

	std::function<int(int,int)> GetRandomIndex = [match, &currentResultOfRound, database](int begin,int end)
	{
		return  tbsg::gameplay::GetRandomIntInRange(begin, end - 1, match->gameSeed);
	};
	lua.SetFunction("GetRandomIndex", GetRandomIndex);
    /**
    * @brief Remove random cards from hand.
    *
    * @param player you want to use.
    *
    * @param amount of cards you want to remove.
    *
    * @return void
    */
    std::function<void(int, int)> RemoveRandomCardsFromHand = [match, &currentResultOfRound, database](int playerID, int amount)
    {
        assert(match != nullptr);

        ptl::vector<Card*> hand;
        for (int i = 0; i < match->playerHands[playerID].size(); ++i)
        {
            hand.push_back(database->GetCard(match->playerHands[playerID][i]));
        }

        for (int i = 0; i < amount; ++i)
        {
            if (hand.size() > 0)
            {
                int randomIndex = tbsg::gameplay::GetRandomIntInRange(0, hand.size() - 1, match->gameSeed);
                match->playerHands[playerID].erase(match->playerHands[playerID].begin() + randomIndex);

                tbsg::Change change{ tbsg::EffectChange::Hand_RemoveCard, static_cast<int>(match->playerHands[playerID][randomIndex]), static_cast<unsigned int>(playerID) };
                currentResultOfRound.results.push_back(change);
            }
        }
    };
    lua.SetFunction("RemoveRandomCardsFromHand", RemoveRandomCardsFromHand);

    /**
    * @brief Remove a card from Discard by Card.
    *
    * @param player you want to use.
    *
    * @param card you want to remove.
    *
    * @return void
    */
    std::function<void(int, Card*)> RemoveCardFromDiscard = [match, &currentResultOfRound](int playerID, Card* card)
    {

		auto cardId = card->id;
		auto res = std::find_if(match->playerDiscards[playerID].begin(), match->playerDiscards[playerID].end(),[&cardId](unsigned int id)
		{
			return 	cardId == id;
		});
		if(res != match->playerDiscards[playerID].end())
		{
			tbsg::Change change{ tbsg::EffectChange::Discard_RemoveCard, static_cast<int>(cardId), static_cast<unsigned int>(playerID) };
			match->playerDiscards[playerID].erase(res);
		}
		/*
        for (auto discradValue : match->playerDiscards[playerID])
        {
            if (discradValue == card->id)
            {
                tbsg::Change change{ tbsg::EffectChange::Discard_RemoveCard, static_cast<int>(discradValue), static_cast<unsigned int>(playerID) };
                currentResultOfRound.results.push_back(change);

                match->playerDiscards[playerID].erase(discradValue);
                return;
            }
        }*/
    };
    lua.SetFunction("RemoveCardFromDiscard", RemoveCardFromDiscard);


    /**
    * @brief Remove a card from deck by index.
    *
    * @param player you want to use.
    *
    * @param index of the card you want to remove.
    *
    * @return void
    */
    std::function<void(int, int)> RemoveCardFromDeck = [match, &currentResultOfRound](int playerID, int index) 
    {
        assert(match != nullptr);
        if (match->playerDecks[playerID].size() > index);
        {
            tbsg::Change change{ tbsg::EffectChange::Deck_RemoveCard, static_cast<int>(match->playerDecks[playerID][index]), static_cast<unsigned int>(playerID) };
            currentResultOfRound.results.push_back(change);

            match->playerDecks[playerID].erase(match->playerDecks[playerID].begin() + index);
        }
    };
    lua.SetFunction("RemoveCardFromDeck", RemoveCardFromDeck);

    /**
    * @brief Remove a card from deck by name.
    *
    * @param player you want to use.
    *
    * @param name of the card you want to remove.
    *
    * @return void
    */
    std::function<void(int, ptl::string)> RemoveCardFromDeckByName = [match, &currentResultOfRound, database](int playerID, ptl::string cardName) 
    {
        assert(match != nullptr);

        for (int i = 0; i < match->playerDecks[playerID].size(); ++i)
        {
            if (database->GetCard(match->playerDecks[playerID][i])->meta.name == cardName)
            {
                tbsg::Change change{ tbsg::EffectChange::Deck_RemoveCard, static_cast<int>(match->playerDecks[playerID][i]), static_cast<unsigned int>(playerID) };
                currentResultOfRound.results.push_back(change);

                match->playerDecks[playerID].erase(match->playerDecks[playerID].begin() + i);
            }
        }
    };
    lua.SetFunction("RemoveCardFromDeckByName", RemoveCardFromDeckByName);




    /**
     * @brief Draw cards from deck.
     *
     * @param player you want to use.
     *
     * @param amount of cards you want to draw.
     *
     * @return void
     */
    std::function<void(int, int)> DrawCardFromDeck = [match, database, &currentResultOfRound](int playerID, int amount) 
    {
        assert(match != nullptr && database != nullptr);

        for (size_t i = 0; i < amount; ++i)
        {
            if (match->playerDecks[playerID].size() > 0)
            {
                unsigned int cardID = match->playerDecks[playerID].back();
                match->playerHands[playerID].push_back(std::move(cardID));
                match->playerDecks[playerID].pop_back();

                tbsg::Change change{ tbsg::EffectChange::Deck_DrawCard, static_cast<int>(cardID), static_cast<unsigned int>(playerID) };
                currentResultOfRound.results.push_back(change);
            }
        }
    };
    lua.SetFunction("DrawCardFromDeck", DrawCardFromDeck);

    /**
     * @brief Draw cards from deck by name.
     *
     * @param player you want to use.
     *
     * @param amount of cards you want to draw.
     *
     * @param name of the card.
     *
     * @return void
     */
    std::function<void(int, int, ptl::string)> DrawNameCardFromDeck = [match, database, &currentResultOfRound](int playerID, int amount, ptl::string name) 
    {
        assert(match != nullptr && database != nullptr);

        auto cardsAdded = 0;

        auto& playerDeck = match->playerDecks[playerID];
        for (auto it = playerDeck.rbegin(); it != playerDeck.rend(); ++it)
        {
            if (cardsAdded < amount)
            {
                unsigned int cardID = *it;
                if (database->GetCard(cardID)->meta.name == name)
                {
                    match->playerHands[playerID].push_back(cardID);
                    playerDeck.erase(--(it.base()));
                    cardsAdded++;

                    tbsg::Change change{ tbsg::EffectChange::Deck_DrawCard, static_cast<int>(cardID), static_cast<unsigned int>(playerID) };
                    currentResultOfRound.results.push_back(change);
                }
            }
            else
            {
                break;
            }
        }
    };
    lua.SetFunction("DrawNameCardFromDeck", DrawNameCardFromDeck);



    /**
    * @brief Gets the current monster the players are facing
    *
    * @param Card you want to use.
    *
    * @return Card*
    */
    std::function<MonsterCard*()> GetMonster = [&monsterCards]()
    {
        if (!monsterCards.empty())
        {
            return &monsterCards[0];
        }
        else
        {
            MonsterCard* card = nullptr;
            return card;
        }
    };
    lua.SetFunction("GetMonster", GetMonster);


    /**
    * @brief Gets monsterTrait of the monster
    *
    * @param MonsterCard you want to get it from
    *
    * @return int
    */
    std::function<int(MonsterCard*)> GetMonsterTrait = [](MonsterCard* card)
    {
        return card->data.monsterTrait;       
    };
    lua.SetFunction("GetMonster", GetMonster);

    // Functions related to Card Health

    /**
     * @brief Gets maXhealth of a card
     *
     * @param Card you want to use.
     *
     * @return int
     */
    std::function<int(MonsterCard*)> GetCardMaxHealth = [match, currentPlayer](MonsterCard* card)
    {
        if (card == nullptr)
        {
            return match->heroes[!currentPlayer].maxHealth;
        }
        return card->data.maxHealth;
    };
    lua.SetFunction("GetCardMaxHealth", GetCardMaxHealth);


    /**
    * @brief Gets ID of a PlayerCard
    *
    * @param Card you want to use.
    *
    * @return int
    */
    std::function<int(Card*)> GetPlayerCardID = [match, currentPlayer](Card* card)
    {
        return card->id;
    };
    lua.SetFunction("GetPlayerCardID", GetPlayerCardID);

    /**
    * @brief Gets ID of a MonsterCard
    *
    * @param Card you want to use.
    *
    * @return int
    */
    std::function<int(MonsterCard*)> GetMonsterCardID = [match, currentPlayer](MonsterCard* card)
    {
        return card->id;
    };
    lua.SetFunction("GetMonsterCardID", GetMonsterCardID);


    /**
     * @brief Gets health of a card
     *
     * @param Card you want to use.
     *
     * @return int
     */
    std::function<int(MonsterCard*)> GetCardHealth = [match, currentPlayer](MonsterCard* card)
    {
        if (card == nullptr)
        {
            return match->heroes[!currentPlayer].health;
        }
        return card->data.health;
    };
    lua.SetFunction("GetCardHealth", GetCardHealth);

    /**
     * @brief Sets health of a card
     *
     * @param Card you want to use.
     *
     * @param New health of a card.
     *
     * @return void
     */
    std::function<void(MonsterCard*, int)> SetCardHealth = [&currentResultOfRound, monsterCards, match, currentPlayer, SetHeroHealth](MonsterCard* card, int health)
    {
        if (card != nullptr)
        {
            if (card->data.health != 0)
            {
                AddChangeCard(card, monsterCards,  currentResultOfRound, health - card->data.health, tbsg::EffectChange::Card_Health);
            }

            if (health < 0)
            {
                card->data.health = 0;
            }
            else
            {
                card->data.health = health;
            }
        }
        else
        {
            SetHeroHealth(&match->heroes[!currentPlayer], health);
        }
    };
    lua.SetFunction("SetCardHealth", SetCardHealth);


    /**
     * @brief Add health to a card.
     *
     * @param Card you want to use.
     *
     * @param Amount of health that will be added to a card.
     *
     * @return void
     */
    std::function<void(MonsterCard*, int)> AddCardHealth = [&currentResultOfRound, monsterCards, match, currentPlayer, AddHeroHealth](MonsterCard* card, int health)
    {
        if (card != nullptr)
        {
            int newHealth = card->data.health + health;

            if (newHealth <= 0)
            {
                AddChangeCard(card, monsterCards,  currentResultOfRound, card->data.health * -1, tbsg::EffectChange::Card_Health);

                card->data.health = 0;
            }
            else if (newHealth > card->data.maxHealth)
            {
                AddChangeCard(card, monsterCards, currentResultOfRound, card->data.maxHealth - card->data.health, tbsg::EffectChange::Card_Health);

                card->data.health = card->data.maxHealth;
            }
            else
            {
                AddChangeCard(card, monsterCards,  currentResultOfRound, health, tbsg::EffectChange::Card_Health);

                card->data.health += health;
            }
        }
        else
        {
            AddHeroHealth(&match->heroes[!currentPlayer], health);
        }
    };
    lua.SetFunction("AddCardHealth", AddCardHealth);

    /**
     * @brief Steal health from a card and give it to a hero.
     *
     * @param Card you want to use.
     *
     * @param Hero that gets the Health.
     *
     * @param Amount of health that will be stolen.
     *
     * @return void
     */
    std::function<void(MonsterCard*, Hero*, int)> StealCardHealth = [match, &currentResultOfRound, monsterCards, currentPlayer, StealHeroHealth](MonsterCard* target, Hero* receiver, int amount)
    {
        if (target != nullptr && receiver != nullptr)
        {
            if (target->data.health >= amount)
            {
                AddChangeCard(target, monsterCards,  currentResultOfRound, amount * -1, tbsg::EffectChange::Card_Health);

                target->data.health -= amount;

                if (receiver->health + amount > receiver->maxHealth)
                {
                    AddChangeHero(receiver, match, currentResultOfRound, receiver->maxHealth - receiver->health, tbsg::EffectChange::Hero_Health);
                    receiver->health = receiver->maxHealth;
                }
                else
                {
                    AddChangeHero(receiver, match, currentResultOfRound, amount, tbsg::EffectChange::Hero_Health);
                    receiver->health += amount;
                }
            }
            else
            {
                if (receiver->health + amount > receiver->maxHealth)
                {
                    AddChangeHero(receiver, match, currentResultOfRound, receiver->maxHealth - receiver->health, tbsg::EffectChange::Hero_Health);
                    receiver->health = receiver->maxHealth;
                }
                else
                {
                    AddChangeHero(receiver, match, currentResultOfRound, target->data.health, tbsg::EffectChange::Hero_Health);
                    receiver->health += target->data.health;
                }

                AddChangeCard(target, monsterCards, currentResultOfRound, target->data.health * -1, tbsg::EffectChange::Card_Health);
                target->data.health = 0;
            }
        }
        else
        {
            StealHeroHealth(&match->heroes[!currentPlayer], receiver, amount);
        }
    };
    lua.SetFunction("StealCardHealth", StealCardHealth);


    /**
     * @brief Gets health of a card
     *
     * @param Card you want to use.
     *
     * @return int
     */
    std::function<int(MonsterCard*)> GetCardArmor = [match, currentPlayer](MonsterCard* card)
    {
        if (card == nullptr)
        {
            return match->heroes[!currentPlayer].health;
        }
        return card->data.armor;
    };
    lua.SetFunction("GetCardArmor", GetCardArmor);

    /**
     * @brief Sets health of a card
     *
     * @param Card you want to use.
     *
     * @param New health of a card.
     *
     * @return void
     */
    std::function<void(MonsterCard*, int)> SetCardArmor = [&currentResultOfRound, monsterCards, match, currentPlayer, SetHeroArmor](MonsterCard* card, int armor)
    {
        if (card != nullptr)
        {
            if (card->data.armor != 0)
            {
                AddChangeCard(card, monsterCards, currentResultOfRound, armor - card->data.armor, tbsg::EffectChange::Card_Armor);
            }

            if (armor < 0)
            {
                card->data.armor = 0;
            }
            else
            {
                card->data.armor = armor;
            }
        }
        else
        {
            SetHeroArmor(&match->heroes[!currentPlayer], armor);
        }
    };
    lua.SetFunction("SetCardArmor", SetCardArmor);


    /**
     * @brief Add health to a card.
     *
     * @param Card you want to use.
     *
     * @param Amount of health that will be added to a card.
     *
     * @return void
     */
    std::function<void(MonsterCard*, int)> AddCardArmor = [&currentResultOfRound, monsterCards, match, currentPlayer, AddHeroArmor](MonsterCard* card, int armor)
    {
        if (card != nullptr)
        {
            int newArmor = card->data.armor + armor;

            if (newArmor <= 0)
            {
                AddChangeCard(card, monsterCards, currentResultOfRound, card->data.armor * -1, tbsg::EffectChange::Card_Armor);

                card->data.armor = 0;
            }
            else
            {
                AddChangeCard(card, monsterCards, currentResultOfRound, armor, tbsg::EffectChange::Card_Armor);

                card->data.armor += armor;
            }
        }
        else
        {
            AddHeroArmor(&match->heroes[!currentPlayer], armor);
        }
    };
    lua.SetFunction("AddCardArmor", AddCardArmor);

    /**
     * @brief Steal health from a card and give it to a hero.
     *
     * @param Card you want to use.
     *
     * @param Hero that gets the Health.
     *
     * @param Amount of health that will be stolen.
     *
     * @return void
     */
    std::function<void(MonsterCard*, Hero*, int)> StealCardArmor = [match, &currentResultOfRound, monsterCards, currentPlayer, StealHeroArmor](MonsterCard* target, Hero* receiver, int amount)
    {
        if (target != nullptr && receiver != nullptr)
        {
            if (target->data.armor >= amount)
            {
                AddChangeCard(target, monsterCards, currentResultOfRound, amount * -1, tbsg::EffectChange::Card_Armor);

                target->data.armor -= amount;

                AddChangeHero(receiver, match, currentResultOfRound, amount, tbsg::EffectChange::Hero_Armor);
                receiver->armor += amount;
            }
            else
            {     
                AddChangeHero(receiver, match, currentResultOfRound, target->data.armor, tbsg::EffectChange::Hero_Armor);
                receiver->health += target->data.armor;
                
                AddChangeCard(target, monsterCards, currentResultOfRound, target->data.armor * -1, tbsg::EffectChange::Card_Armor);
                target->data.armor = 0;
            }
        }
        else if (receiver != nullptr)
        {
            StealHeroArmor(&match->heroes[!currentPlayer], receiver, amount);
        }
    };
    lua.SetFunction("StealCardArmor", StealCardArmor);



    /**
     * @brief Gets the reward of a card.
     *
     * @param Card you want to use.
     *
     * @return Reward*
     */
    std::function<Reward*(MonsterCard*)> GetCardReward = [](MonsterCard* card)
    {
        assert(card != nullptr);
        return card->data.reward[0];
    };
    lua.SetFunction("GetCardReward", GetCardReward);


    /**
     * @brief Checks if the deck has a card.
     *
     * @param Card you want to use.
     *
     * @param Deck you want to use.
     *
     * @return bool
     */
    std::function<bool(Card*, Deck)> HasCard = [](Card* card, Deck deck)
    {
        if (card != nullptr)
        {
            for (size_t i = 0; i < deck.cards.size(); i++)
            {
                if (deck.cards[i]->id == card->id)
                {
                    return true;
                }
            }
        }
        return false;
    };
    lua.SetFunction("HasCard", HasCard);

    /**
     * @brief Gets a card from a deck and removes it.
     *
     * @param index of the card you want to get.
     *
     * @param Deck you want to use.
     *
     * @return Card*
     */
    std::function<Card*(int)> GetCard = [database](unsigned int ID)
    {       
        return database->GetCard(ID);
    };
    lua.SetFunction("GetCard", GetCard);

	std::function<void(ptl::string)> PlaySound = [currentPlayer, &currentResultOfRound](ptl::string soundName)
	{
        tbsg::Change change{ tbsg::EffectChange::Play_Sound, static_cast<int>(HashString(ptl::string(soundName))), static_cast<unsigned int>(currentPlayer) };
		currentResultOfRound.results.push_back(change);
	};

	lua.SetFunction("PlaySound", PlaySound);
}

void tbsg::scripting::AddChangeCard(MonsterCard * card, ptl::vector<tbsg::MonsterCard> monsterCards, ResultOfRound& currentResultOfRound, int amount, EffectChange changeType)
{
	tbsg::Change change;
	change.change = amount;
	change.changeType = changeType;

	for (int cardID = 0; cardID < monsterCards.size(); ++cardID)
	{
		if (&monsterCards[cardID] == card)
		{
			change.index = cardID;
		}
	}
	currentResultOfRound.results.push_back(change);
}

int tbsg::scripting::AddChangeHero(Hero * hero, MatchData * match, ResultOfRound & currentResultOfRound, int amount, EffectChange changeType)
{
    tbsg::Change change;
    change.change = amount;
    change.changeType = changeType;

    if (&match->heroes[0] == hero)
    {
        change.index = 0;
    }
    else
    {
        change.index = 1;
    }

    currentResultOfRound.results.push_back(change);
    return change.index;
}

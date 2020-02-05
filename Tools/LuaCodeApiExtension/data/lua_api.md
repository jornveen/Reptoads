# Lua API 
## function GetMatch ()

**Brief:**   Gets the current match.

**Returns:**   Match

## function GetDeck (int)

**Brief:**   Gets deck from array index is number in array.

**Returns:**   Deck

## function GetHand (int)

**Brief:**   Gets hand by index.

**Parameters:**
- player you want to use.

**Returns:**   vector<Card>

## function GetRival ()

**Brief:**   Gets the rivalHero of the game.

**Returns:**   Hero

## function GetHero ()

**Brief:**   Gets the playerHero of the game.

**Returns:**   Hero

## function GetHeroByID (int)

**Brief:**   Gets the Hero of the game by index.

**Parameters:**
- the ID of the hero you want to get

**Returns:**   Hero

## function AreHeroesAtBoss ()

**Brief:**   Returns true when you are at the boss

**Returns:**   boolean

## function AreHeroesAtEndFight ()

**Brief:**   Returns true when you are at the end fight

**Returns:**   boolean

## function PlayerFightDuration ()

**Brief:**   Gets the number of turns the player is in combat with the monster. Resets if it kill it and goes to the next.

**Returns:**   int

## function SetFirstPlayerFlag (int)

**Brief:**   Sets the first player tag.

**Parameters:**
- player you want to Set the tag to

**Returns:**   void

## function SwitcheFirstPlayerFlag ()

**Brief:**   Switches the first player tag.

**Returns:**   void

## function GetHeroResource (Hero*)

**Brief:**   Gets resource of a hero.

**Parameters:**
- Hero you want to use.

**Returns:**   int

## function AddHeroResource (Hero*, int)

**Brief:**   Add resources to a hero.

**Parameters:**
- Hero you want to use.- Amount of resource you want to add to the hero.

**Returns:**   void

## function SetHeroResource (Hero*, int)

**Brief:**   Sets the resources of a hero.

**Parameters:**
- Hero you want to use.- New amount of resource.

**Returns:**   void

## function StealHeroResource (Hero*, Hero*, int)

**Brief:**   Steals resources from one hero and gives it to another hero.

**Parameters:**
- Hero you want to steal it from.- Hero that gets the resources.- Amount of resources you want to steal.

**Returns:**   void

## function GetHeroHealth (Hero*)

**Brief:**   Gets health of a hero.

**Parameters:**
- Hero you want to use.

**Returns:**   int

## function AddHeroHealth (Hero*, int)

**Brief:**   Add health to a hero.

**Parameters:**
- Hero you want to use.- Amount of health you want to add to the hero.

**Returns:**   void

## function SetHeroHealth (Hero*, int)

**Brief:**   Sets the health of a hero.

**Parameters:**
- Hero you want to use.- New amount of health.

**Returns:**   void

## function StealHeroHealth (Hero*, Hero*, int)

**Brief:**   Steals health from one hero and gives it to another hero.

**Parameters:**
- Hero you want to steal it from.- Hero that gets the health.- Amount of health you want to steal.

**Returns:**   void

## function GetHeroArmor (Hero*)

**Brief:**   Gets armor of a hero.

**Parameters:**
- Hero you want to use.

**Returns:**   int

## function AddHeroArmor (Hero*, int)

**Brief:**   Add armor to a hero.

**Parameters:**
- Hero you want to use.- Amount of armor you want to add to the hero.

**Returns:**   void

## function SetHeroArmor (Hero*, int)

**Brief:**   Sets the armor of a hero.

**Parameters:**
- Hero you want to use.- New amount of armor.

**Returns:**   void

## function StealHeroArmor (Hero*, Hero*, int)

**Brief:**   Steals armor from one hero and gives it to another hero.

**Parameters:**
- Hero you want to steal it from.- Hero that gets the armor.- Amount of armor you want to steal.

**Returns:**   void

## function GetHeroAttack (Hero*)

**Brief:**   Gets attack of a hero.

**Parameters:**
- Hero you want to use.

**Returns:**   int

## function AddHeroAttack (Hero*, int)

**Brief:**   Add attack to a hero.

**Parameters:**
- Hero you want to use.- Amount of attack you want to add to the hero.

**Returns:**   void

## function SetHeroAttack (Hero*, int)

**Brief:**   Sets the attack of a hero.

**Parameters:**
- Hero you want to use.- New amount of attack.

**Returns:**   void

## function StealHeroAttack (Hero*, Hero*, int)

**Brief:**   Steals attack from one hero and gives it to another hero.

**Parameters:**
- Hero you want to steal it from.- Hero that gets the attack.- Amount of attack you want to steal.

**Returns:**   void

## function GetHeroTotalAttack (Hero*)

**Brief:**   Gets the total attack of a hero.

**Parameters:**
- Hero you want to use.

**Returns:**   int

## function GetHeroWeapon (Hero*)

**Brief:**   Gets weapon of a hero.

**Parameters:**
- Hero you want to use.

**Returns:**   Weapon

## function SetHeroWeapon (Hero*, Weapon*)

**Brief:**   Sets the weapon of a hero.

**Parameters:**
- Hero you want to use.- New weapon of hero.

**Returns:**   void

## function StealHeroWeapon (Hero*, Hero*)

**Brief:**   Steals the weapon from one hero and gives it to another hero.

**Parameters:**
- Hero you want to steal it from.- Hero that gets the weapon.

**Returns:**   void

## function CreateHeroWeapon (int, int)

**Brief:**   Creates a weapon and returns it.

**Parameters:**
- Attack of the weapon.- durability of the weapon.

**Returns:**   Weapon

## function GetWeaponAttack (Hero*)

**Brief:**   Gets attack of a weapon.

**Parameters:**
- Hero you want to use.

**Returns:**   int

## function AddWeaponAttack (Hero*, int)

**Brief:**   Add attack to a weapon.

**Parameters:**
- Hero you want to use.- Amount of attack you want to add to the weapon.

**Returns:**   void

## function SetWeaponAttack (Hero*, int)

**Brief:**   Sets the attack of a weapon.

**Parameters:**
- Hero you want to use.- New amount of attack.

**Returns:**   void

## function GetWeaponDurability (Hero*)

**Brief:**   Gets durability of a weapon.

**Parameters:**
- Hero you want to use.

**Returns:**   int

## function AddWeaponDurability (Hero*, int)

**Brief:**   Add durability to a weapon.

**Parameters:**
- Hero you want to use.- Amount of durability you want to add to the weapon.

**Returns:**   void

## function SetWeaponDurability (Hero*, int)

**Brief:**   Sets the durability of a weapon.

**Parameters:**
- Hero you want to use.- New amount of durability.

**Returns:**   void

## function GetCurrentMonster ()

**Brief:**   Gets the monster you are currently facing

**Returns:**   Hero

## function GetPlayedCard (int)

**Brief:**   Gets the played card by player and index.

**Parameters:**
- player you want to get the played cards from.- number of the card you want to check.

**Returns:**   Card

## function GetCardFromHand (int, int)

**Brief:**   Gets a card from hand.

**Parameters:**
- player you want to use.- index of the card you want to get.

**Returns:**   Card

## function GetHandSize (int)

**Brief:**   Gets the amount of cards you have in your hand.

**Parameters:**
- player you want to use.

**Returns:**   int

## function GetDeckSize (int)

**Brief:**   Gets the amount of cards you have in your hand.

**Parameters:**
- player you want to use.

**Returns:**   int

## function GetDiscardSize (int)

**Brief:**   Gets the amount of cards you have in your hand.

**Parameters:**
- player you want to use.

**Returns:**   int

## function GetCardInHandByName (int, string)

**Brief:**   Gets the cardIndex of a card in your hand by name.

**Parameters:**
- player you want to use.- name of the card.

**Returns:**   int

## function GetAmountOfCardsInHandByName (int, string)

**Brief:**   Gets the amount of cards you have in your hand by name.

**Parameters:**
- player you want to use.- name of the card.

**Returns:**   int

## function GetCurrentplayerID ()

**Brief:**   Gets the current playerID.

**Returns:**   int

## function GetCurrentRivalID ()

**Brief:**   Gets the current rivalID.

**Returns:**   int

## function AddCardToDeckByType (int, string)

**Brief:**   Adds a card with a specific card type to the player hand.

**Parameters:**
- player you want to use.- Type of the card.

**Returns:**   void

## function AddCardToHandByType (int, string)

**Brief:**   Adds a card with a specific card type to the player hand.

**Parameters:**
- player you want to use.- Type of the card.

**Returns:**   void

## function s not supported anymore
    //std::function<void(int, ptl::string)> AddCardToHand ()

**Brief:**   Adds a card to hand by player.

**Parameters:**
- player you want to use.- name of the card.

**Returns:**   void

## function s not supported anymore
    //std::function<void(int, ptl::string, int)> AddCardsToHand ()

**Brief:**   Adds an amount of cards to hand.

**Parameters:**
- player you want to use.- name of the card.- amount of cards you want to add

**Returns:**   void

## function RemoveCardFromHand (int, int)

**Brief:**   Removes a card from hand by index.

**Parameters:**
- player you want to use.- index of the card you want to remove.

**Returns:**   void

## function RemoveAllCardsFromHand (int)

**Brief:**   Removes all card from a player.

**Parameters:**
- player you want to use.

**Returns:**   void

## function RemoveCardFromHandByName (int, string)

**Brief:**   Removes a card from hand by name.

**Parameters:**
- player you want to use.- name of the card.

**Returns:**   void

## function RemoveCardsFromHandByName (int, string, int)

**Brief:**   Removes all card from hand by name.

**Parameters:**
- player you want to use.- name of the card.

**Returns:**   void

## function RemoveRandomCardFromHandByName (int, string)

**Brief:**   Removes a random card from hand by name.

**Parameters:**
- player you want to use.- name of the card.

**Returns:**   void

## function GetDeckCard (int, int)

**Brief:**   Gets a card from deck by index.

**Parameters:**
- player you want to use.- index of the card you want to get.

**Returns:**   Card

## function GetCardInDeckByName (int, string)

**Brief:**   Gets a card from deck by name

**Parameters:**
- player you want to use.- name of the card.

**Returns:**   int

## function s not supported anymore
    //std::function<void(int, ptl::string)> AddCardToDeck ()

**Brief:**   Adds a card to the deck

**Parameters:**
- player you want to use.- name of the card.

**Returns:**   void

## function s not supported anymore
    //std::function<void(int, ptl::string, int)> AddCardsToDeck ()

**Brief:**   Adds a card to the deck by name.

**Parameters:**
- player you want to use.- name of the card.- amount of cards you want to add.

**Returns:**   void

## function RemoveRandomCardFromDeck (int)

**Brief:**   Removes random card from deck.

**Parameters:**
- player you want to use.

**Returns:**   void

## function RemoveRandomCardsFromDeck (int, int)

**Brief:**   Remove random cards from deck.

**Parameters:**
- player you want to use.- amount of cards you want to remove.

**Returns:**   void

## function RemoveRandomCardFromHand (int)

**Brief:**   Remove random card from hand.

**Parameters:**
- player you want to use.

**Returns:**   void

## function RemoveRandomCardsFromHand (int, int)

**Brief:**   Remove random cards from hand.

**Parameters:**
- player you want to use.- amount of cards you want to remove.

**Returns:**   void

## function RemoveCardFromDeck (int, int)

**Brief:**   Remove a card from deck by index.

**Parameters:**
- player you want to use.- index of the card you want to remove.

**Returns:**   void

## function RemoveCardFromDeckByName (int, string)

**Brief:**   Remove a card from deck by name.

**Parameters:**
- player you want to use.- name of the card you want to remove.

**Returns:**   void

## function DrawCardFromDeck (int, int)

**Brief:**   Draw cards from deck.

**Parameters:**
- player you want to use.- amount of cards you want to draw.

**Returns:**   void

## function DrawNameCardFromDeck (int, int, string)

**Brief:**   Draw cards from deck by name.

**Parameters:**
- player you want to use.- amount of cards you want to draw.- name of the card.

**Returns:**   void

## function GetMonster ()

**Brief:**   Gets the current monster the players are facing

**Parameters:**
- Card you want to use.

**Returns:**   Card

## function GetMonsterTrait (MonsterCard*)

**Brief:**   Gets monsterTrait of the monster

**Parameters:**
- MonsterCard you want to get it from

**Returns:**   int

## function GetCardMaxHealth (MonsterCard*)

**Brief:**   Gets maXhealth of a card

**Parameters:**
- Card you want to use.

**Returns:**   int

## function GetCardHealth (MonsterCard*)

**Brief:**   Gets health of a card

**Parameters:**
- Card you want to use.

**Returns:**   int

## function SetCardHealth (MonsterCard*, int)

**Brief:**   Sets health of a card

**Parameters:**
- Card you want to use.- New health of a card.

**Returns:**   void

## function AddCardHealth (MonsterCard*, int)

**Brief:**   Add health to a card.

**Parameters:**
- Card you want to use.- Amount of health that will be added to a card.

**Returns:**   void

## function StealCardHealth (MonsterCard*, Hero*, int)

**Brief:**   Steal health from a card and give it to a hero.

**Parameters:**
- Card you want to use.- Hero that gets the Health.- Amount of health that will be stolen.

**Returns:**   void

## function GetCardArmor (MonsterCard*)

**Brief:**   Gets health of a card

**Parameters:**
- Card you want to use.

**Returns:**   int

## function SetCardArmor (MonsterCard*, int)

**Brief:**   Sets health of a card

**Parameters:**
- Card you want to use.- New health of a card.

**Returns:**   void

## function AddCardArmor (MonsterCard*, int)

**Brief:**   Add health to a card.

**Parameters:**
- Card you want to use.- Amount of health that will be added to a card.

**Returns:**   void

## function StealCardArmor (MonsterCard*, Hero*, int)

**Brief:**   Steal health from a card and give it to a hero.

**Parameters:**
- Card you want to use.- Hero that gets the Health.- Amount of health that will be stolen.

**Returns:**   void

## function GetCardReward (MonsterCard*)

**Brief:**   Gets the reward of a card.

**Parameters:**
- Card you want to use.

**Returns:**   Reward

## function HasCard (Card*, Deck)

**Brief:**   Checks if the deck has a card.

**Parameters:**
- Card you want to use.- Deck you want to use.

**Returns:**   bool

## function GetCard (int)

**Brief:**   Gets a card from a deck and removes it.

**Parameters:**
- index of the card you want to get.- Deck you want to use.

**Returns:**   Card


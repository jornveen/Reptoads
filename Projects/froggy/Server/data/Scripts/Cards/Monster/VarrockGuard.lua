require('CombatLibrary')
function OnCardPlay(a_Card)
    
    hero = GetHero()
    Combat(hero,a_Card)

    health = GetCardHealth(a_Card)
    if (health <= 0) then
        rival = GetRival()
        rivalResource = GetHeroResource(rival)
        AddHeroArmor(hero,rivalResource)
    end
end
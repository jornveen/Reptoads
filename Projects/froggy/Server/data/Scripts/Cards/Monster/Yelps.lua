require('CombatLibrary')
function OnCardPlay(a_Card)
    
    hero = GetHero()
    Combat(hero,a_Card)

    health = GetCardHealth(a_Card)
    if (health <= 0) then
        resource = GetHeroResource(hero)
        if(resource <= 8) then
            AddHeroResource(resource)
        end
    end
end
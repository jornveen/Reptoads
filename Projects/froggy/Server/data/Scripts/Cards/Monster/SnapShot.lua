function OnCardPlay(a_Card)
    
    playerHero  = GetHero()
    rivalHero   = GetRival()

    AddHeroHealth(rivalHero, (GetWeaponAttack(playerHero) + 1) * -1)

    print("Rival took " .. (GetWeaponAttack(playerHero) + 1) .. " damage!")
end
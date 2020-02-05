function OnCardPlay(a_Card)
    
    playerIndex = GetCurrentPlayerID()
    cardIndex = GetCardInHandByFamily(playerIndex, "Ship")

    if (cardIndex ~= -1) then
        playerHero = GetHero()
        AddHeroResource(playerHero,5)
    end
end
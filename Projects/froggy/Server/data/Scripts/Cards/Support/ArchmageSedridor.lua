function OnCardPlay(a_Card)
    
    rivalIndex = GetCurrentRivalID()
    currentCardIndex = GetCardPlayedIndex()
    nextCreature = GetNextCreature(rivalIndex, currentCardIndex)

    if (nextCreature ~= nil) then
        AddCardHealth(nextCreature, 2)
        AddCardAttack(nextCreature, 2)
    end
end
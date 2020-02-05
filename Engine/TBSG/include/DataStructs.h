#pragma once

#include "memory/Containers.h"
#include <functional>

namespace tbsg {
    //struct Card;
    /*! \addtogroup Gameplay
     *  @{
     */
    /**
     * \brief The represenation of the current match on the Client Side
     */
    struct Round
    {
        ptl::vector<tbsg::Card*> hand;
        ptl::vector<tbsg::Card*> playedCards;
    };



    /*! @} */
}

//struct Chapter
//{
//    //ptl::vector<Model*> models;
//    //ptl::vector<SpawnPoints> spawn_points;
//};

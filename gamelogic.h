#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include <map.h>

class GameLogic
{
public:
    GameLogic(Map *map);
    
    bool checkWin();
private:
    Map *m_map;
};

#endif // GAMELOGIC_H

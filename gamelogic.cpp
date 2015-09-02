#include "gamelogic.h"

GameLogic::GameLogic(Map *map)
{
    m_map = map;
}

const int dx[8] = {1, 1, 1, 0, 0, -1, -1, -1};
const int dy[8] = {-1, 0, 1, -1, 1, -1, 0, 1};

bool GameLogic::checkWin() {
    for (int i = 0; i < 15; ++i)
        for (int j = 0; j < 15; ++j) {
            if (m_map->arr[i][j] == -1)
                continue;
            for (int k = 0; k < 8; ++k) {
                int di = i + dx[k];
                int dj = j + dy[k];
                int cur = 1;
                while (di >= 0 && di < 15 && dj >= 0 && dj < 15 && m_map->arr[di][dj] == m_map->arr[i][j]) {
                    ++cur;
                    di = di + dx[k];
                    dj = dj + dy[k];
                }
                if (cur >= 5) {
                    return true;
                }
            }
        }
    return false;
}

#ifndef MAP_H
#define MAP_H

class Map {
public:
    int arr[15][15];
    enum Player{
        Black = 0,
        White = 1
    }curPlayer;
    
    Map() {
        memset(arr, -1, sizeof arr);
        curPlayer = Black;
    }
};

#endif // MAP_H


#ifndef MAP_H
#define MAP_H

#include <cstring>
#include <QString>
#include <QFile>
#include <QPoint>
#include <vector>
#include <QTextStream>

class Map {
public:
    int arr[15][15];
    std::vector<QPoint> place;
    enum Player{
        Black = 0,
        White = 1
    }curPlayer;
    
    Map() {
        memset(arr, -1, sizeof arr);
        place.clear();
        curPlayer = Black;
    }
    void load(QString fileName) {
        QFile file(fileName);
        file.open(QIODevice::ReadOnly);
        QTextStream fin(&file);
        int flag;
        fin >> flag;
        if (flag == 0)
            curPlayer = Black;
        else
            curPlayer = White;
        for (int i = 0; i < 15; ++i)
            for (int j = 0; j < 15; ++j)
                fin >> arr[i][j];
        int n, x, y;
        fin >> n;
        place.clear();
        for (int i = 0; i < n; ++i) {
            fin >> x >> y;
            place.push_back(QPoint(x, y));
        }
    }
    void save(QString fileName) {
        QFile file(fileName);
        file.open(QIODevice::WriteOnly);
        QTextStream fout(&file);
        fout << (int)curPlayer << endl;
        for (int i = 0; i < 15; ++i) {
            for (int j = 0; j < 15; ++j)
                fout << arr[i][j] << " ";
            fout << endl;
        }
        fout << place.size() << endl;
        for (int i = 0; i < place.size(); ++i) {
            fout << place[i].x() << " " << place[i].y() << endl;
        }
    }

    void pop(int player) {
        while (true) {
            QPoint p = place[(int)place.size() - 1];
            place.pop_back();
            if (arr[p.x()][p.y()] == player) {
                arr[p.x()][p.y()] = -1;
                break;
            }else {
                arr[p.x()][p.y()] = -1;
            }
        }
        curPlayer = Player(player);
    }
};

#endif // MAP_H


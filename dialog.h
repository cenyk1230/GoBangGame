#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTcpServer>
#include <QTcpSocket>
#include <QLineEdit>
#include <QMouseEvent>
#include <map.h>
#include <gamelogic.h>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog() { delete ui; }
    
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void calcSend(int, int);
    void putPoint(int, int);
    void gameOver(bool);
    
public slots:
    void chooseFightModelWidget();
    void networkGameWidget();
    void prepareGameWidget();
    void createHost();
    void joinHost();
    void cancelCreateHost();
    void acceptConnection();
    void beginJoinHost();
    void cancelJoinHost();
    void waitForConnect();
    void softKeyInput(QString);
    void calcRead();
    
private:
    Ui::Dialog *ui;
    int m_num;
    bool isHost;
    bool isGameWidget;
    bool isGameBegin;
    Map::Player player;
    Map *map;
    GameLogic *gameLogic;
    QString myIP;
    QWidget *widget[10];
    QPushButton *networkFightButton, *personAIFightButton;
    QPushButton *createHostButton, *joinHostButton;
    QPushButton *OKWaitButton, *cancelWaitButton;
    QPushButton *OKJoinButton, *cancelJoinButton;
    QPushButton *OKGameOverButton;
    QPushButton *numberButton[10];
    QPushButton *dotButton, *delButton;
    QLineEdit *IPEdit;
    QDialog *createHostDialog, *joinHostDialog, *gameOverDialog;
    QTcpServer *listenSocket;
    QTcpSocket *readWriteSocket;
};

#endif // DIALOG_H

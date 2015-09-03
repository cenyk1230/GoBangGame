#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTcpServer>
#include <QTcpSocket>
#include <QLineEdit>
#include <QMouseEvent>
#include <QLabel>
#include <QTimer>
#include <QLCDNumber>
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
    void sendMessage(QString);
    void changePlayer();
    void renewTime();
    bool checkNoMyChess();
    
public slots:
    void setNameAvatarWidget();
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
    void chooseAvatarFromFile();
    void setMyInfo();
    void prepareToRead();
    void sendAvatar();
    void sendUsername();
    void prepareToBegin();
    void onTimerOut();
    void requestForQuit();
    void requestForSave();
    void requestForLoad();
    void requestForWithdraw();
    
private:
    Ui::Dialog *ui;
    int m_num;
    bool isHost;
    bool isGameWidget;
    bool isGameBegin;
    bool isOpBegin, isMeBegin;
    int mySingleTime, opSingleTime;
    int myTotalTime, opTotalTime;
    int withDrawTime;
    Map::Player player;
    Map *map;
    GameLogic *gameLogic;
    QString myIP;
    QString myUserName, opUserName, myAvatarFileName, opAvatarFileName;
    QWidget *widget[10], *boardWidget, *myInfoWidget, *opInfoWidget;
    QPushButton *networkFightButton, *personAIFightButton;
    QPushButton *createHostButton, *joinHostButton;
    QPushButton *OKWaitButton, *cancelWaitButton;
    QPushButton *OKJoinButton, *cancelJoinButton;
    QPushButton *OKGameOverButton;
    QPushButton *numberButton[10];
    QPushButton *dotButton, *delButton;
    QPushButton *chooseAvatarButton, *OKchooseButton;
    QPushButton *beginButton, *quitButton, *withdrawButton, *saveButton, *loadButton;
    QPixmap myAvatarPixmap, opAvatarPixmap;
    QLabel *chooseAvatarLabel, *myAvatarLabel, *opAvatarLabel, *myNameLabel, *opNameLabel;
    QLineEdit *chooseNameEdit;
    QLineEdit *IPEdit;
    QLCDNumber *mySingleTimeLCD, *myTotalTimeLCD;
    QLCDNumber *opSingleTimeLCD, *opTotalTimeLCD;
    QTimer *timer;
    QDialog *createHostDialog, *joinHostDialog, *gameOverDialog;
    QTcpServer *listenSocket;
    QTcpSocket *readWriteSocket;
};

#endif // DIALOG_H

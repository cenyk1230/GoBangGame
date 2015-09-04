#include "dialog.h"
#include "ui_dialog.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QHostInfo>
#include <QDebug>
#include <QSignalMapper>
#include <QPainter>
#include <QMessageBox>
#include <QDateTime>
#include <QSound>
#include <QFileDialog>

const static int INIT_WIDTH = 300;
const static int INIT_HEIGHT = 300;
const static int GAME_WIDTH = 840;
const static int GAME_HEIGHT = 670;
const static int LCX = 20;
const static int LCY = 20;
const static int PORT = 8888;

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    this->setFixedSize(INIT_WIDTH, INIT_HEIGHT);
    this->setWindowTitle("GoBangGame");
    m_num = -1;
    isGameWidget = false;
    isGameBegin = false;
    isOpBegin = isMeBegin = false;
    listenSocket = nullptr;
    readWriteSocket = nullptr;
    opUserName = "";
    mySingleTime = opSingleTime = 20;
    myTotalTime = opTotalTime = 0;
    withDrawTime = 0;
    map = new Map();
    timer = new QTimer();
    timer->setInterval(1000);
    gameLogic = new GameLogic(map);
    background.load(":/picture/picture/background.jpg");
    background = background.scaled(QSize(GAME_WIDTH, GAME_HEIGHT), Qt::KeepAspectRatio);
    
    boardWidget = new QWidget(this);
    boardWidget->setFixedSize(600, 600);
    myInfoWidget = new QWidget();
    myInfoWidget->setFixedWidth(200);
    opInfoWidget = new QWidget();
    opInfoWidget->setFixedWidth(200);
    
    setNameAvatarWidget();
}

void Dialog::setNameAvatarWidget() {
    if (m_num != -1) {
        delete widget[m_num];
    }
    m_num = 2;
    widget[m_num] = new QWidget(this);
    widget[m_num]->setFixedSize(INIT_WIDTH, INIT_HEIGHT);
    QVBoxLayout *vt = new QVBoxLayout(widget[m_num]);
    QHBoxLayout *ht = new QHBoxLayout();
    QHBoxLayout *ht2 = new QHBoxLayout();
    QLabel *label = new QLabel("请输入用户名:", widget[m_num]);
    chooseAvatarLabel = new QLabel(widget[m_num]);
    chooseAvatarLabel->setFixedSize(150, 150);
    myAvatarFileName = "1.JPG";
    myAvatarPixmap.load("1.JPG");
    chooseAvatarLabel->setPixmap(myAvatarPixmap);
    chooseNameEdit = new QLineEdit(widget[m_num]);
    chooseAvatarButton = new QPushButton("选择头像", widget[m_num]);
    OKchooseButton = new QPushButton("确定", widget[m_num]);
    vt->addWidget(chooseAvatarLabel, 0, Qt::AlignHCenter);
    vt->addLayout(ht);
    vt->addLayout(ht2);
    ht->addWidget(label, 0, Qt::AlignHCenter);
    ht->addWidget(chooseNameEdit, 0, Qt::AlignHCenter);
    ht2->addWidget(chooseAvatarButton, 0, Qt::AlignHCenter);
    ht2->addWidget(OKchooseButton, 0, Qt::AlignHCenter);
    widget[m_num]->show();
    
    connect(chooseAvatarButton, SIGNAL(clicked(bool)), this, SLOT(chooseAvatarFromFile()));
    connect(OKchooseButton, SIGNAL(clicked(bool)), this, SLOT(setMyInfo()));
    //connect(OKchooseButton, SIGNAL(clicked(bool)), this, SLOT(chooseFightModelWidget()));
}

void Dialog::setMyInfo() {
    //qDebug() << "Set My Info";
    myUserName = chooseNameEdit->text();
    if (myUserName == "") {
        myUserName = "Username";
    }
    //chooseFightModelWidget();
    networkGameWidget();
}

void Dialog::chooseAvatarFromFile() {
    QString st = QFileDialog::getOpenFileName(this, "选取头像", "", "Image Files(*.jpg *.jpeg)");
    QPixmap pixmap(st);
    QSize size(chooseAvatarLabel->width(), chooseAvatarLabel->height());
    QPixmap pixmap2 = pixmap.scaled(size, Qt::KeepAspectRatio);
    if (!pixmap2.isNull()) {
        QStringList info = st.split("/");
        myAvatarFileName = info.at(info.size() - 1);
        myAvatarPixmap = pixmap2;
        chooseAvatarLabel->setPixmap(pixmap2);
    }
}

void Dialog::chooseFightModelWidget() {
    //qDebug() << "chooseFightModelWidget";
    if (m_num != -1) {
        delete widget[m_num];
    }
    m_num = 0;
    this->setFixedSize(INIT_WIDTH, INIT_HEIGHT);
    widget[m_num] = new QWidget(this);
    widget[m_num]->setFixedSize(INIT_WIDTH, INIT_HEIGHT);
    networkFightButton = new QPushButton("网络对战", widget[m_num]);
    personAIFightButton = new QPushButton("人机对战", widget[m_num]);
    QVBoxLayout *vt = new QVBoxLayout(widget[m_num]);
    vt->addWidget(networkFightButton, 0, Qt::AlignHCenter);
    vt->addWidget(personAIFightButton, 0, Qt::AlignHCenter);
    widget[m_num]->show();
    connect(networkFightButton, SIGNAL(clicked(bool)), this, SLOT(networkGameWidget()));
}

void Dialog::networkGameWidget() {
    if (m_num != -1) {
        delete widget[m_num];
    }
    m_num = 1;
    this->setFixedSize(INIT_WIDTH, INIT_HEIGHT);
    widget[m_num] = new QWidget(this);
    widget[m_num]->setFixedSize(INIT_WIDTH, INIT_HEIGHT);
    createHostButton = new QPushButton("创建主机", widget[m_num]);
    joinHostButton = new QPushButton("加入主机", widget[m_num]);
    QVBoxLayout *vt = new QVBoxLayout(widget[m_num]);
    vt->addWidget(createHostButton, 0, Qt::AlignHCenter);
    vt->addWidget(joinHostButton, 0, Qt::AlignHCenter);
    widget[m_num]->show();
    
    connect(createHostButton, SIGNAL(clicked(bool)), this, SLOT(createHost()));
    connect(joinHostButton, SIGNAL(clicked(bool)), this, SLOT(joinHost()));
}

void Dialog::createHost() {
    createHostDialog = new QDialog();
    createHostDialog->setWindowTitle("正在等待客户端加入");
    createHostDialog->setFixedSize(300, 200);
    
    QString localHostName = QHostInfo::localHostName();
    QHostInfo info = QHostInfo::fromName(localHostName);
    foreach(QHostAddress address, info.addresses())
        if(address.protocol() == QAbstractSocket::IPv4Protocol) {
            qDebug() << address.toString();
            QStringList list = address.toString().split(".");
            if (list[0] == "127")// || list[0] == "10")
                continue;
            myIP = address.toString();
            break;
        }
    QLabel *IPLabel = new QLabel("Host IP: ", createHostDialog);
    IPEdit = new QLineEdit(myIP, createHostDialog);
    cancelWaitButton = new QPushButton("取消", createHostDialog);
    OKWaitButton = new QPushButton("连接", createHostDialog);
    QVBoxLayout *vt = new QVBoxLayout(createHostDialog);
    QHBoxLayout *ht = new QHBoxLayout();
    QHBoxLayout *ht2 = new QHBoxLayout();
    vt->addLayout(ht);
    vt->addLayout(ht2);
    ht->addWidget(IPLabel, 0, Qt::AlignHCenter);
    ht->addWidget(IPEdit, 0, Qt::AlignHCenter);
    ht2->addWidget(OKWaitButton, 0, Qt::AlignHCenter);
    ht2->addWidget(cancelWaitButton, 0, Qt::AlignHCenter);
    createHostDialog->show();
    
    connect(OKWaitButton, SIGNAL(clicked(bool)), this, SLOT(waitForConnect()));
    connect(cancelWaitButton, SIGNAL(clicked(bool)), this, SLOT(cancelCreateHost()));

}

void Dialog::cancelCreateHost() {
    if (listenSocket != nullptr) {
        delete listenSocket;
        listenSocket = nullptr;
    }
    OKWaitButton->setEnabled(true);
    //delete createHostDialog;
}

void Dialog::joinHost() {
    joinHostDialog = new QDialog();
    joinHostDialog->setWindowTitle("准备加入主机");
    joinHostDialog->setFixedSize(300, 320);
    
    QVBoxLayout *vt = new QVBoxLayout(joinHostDialog);
    QGridLayout *gt = new QGridLayout();
    QHBoxLayout *ht1 = new QHBoxLayout();
    QHBoxLayout *ht2 = new QHBoxLayout();
    vt->addLayout(ht1);
    vt->addLayout(gt);
    vt->addLayout(ht2);
    
    OKJoinButton = new QPushButton("连接主机", joinHostDialog);
    cancelJoinButton = new QPushButton("取消加入", joinHostDialog);
    ht2->addWidget(OKJoinButton);
    ht2->addWidget(cancelJoinButton);
    
    QLabel *label = new QLabel("请输入主机IP: ", joinHostDialog);
    IPEdit = new QLineEdit(joinHostDialog);
    ht1->addWidget(label);
    ht1->addWidget(IPEdit);
    
    QSignalMapper *m = new QSignalMapper(joinHostDialog);
    for (int i = 0; i < 10; ++i) {
        numberButton[i] = new QPushButton(QString::number(i), joinHostDialog);
        connect(numberButton[i], SIGNAL(clicked(bool)), m, SLOT(map()));
        m->setMapping(numberButton[i], QString::number(i));
        if (i > 0)
            gt->addWidget(numberButton[i], 2 - (i - 1) /  3, (i - 1) % 3);
    }
    dotButton = new QPushButton(".", joinHostDialog);
    delButton = new QPushButton("退格", joinHostDialog);
    gt->addWidget(numberButton[0], 3, 0);
    gt->addWidget(dotButton, 3, 1);
    gt->addWidget(delButton, 3, 2);
    connect(dotButton, SIGNAL(clicked(bool)), m, SLOT(map()));
    connect(delButton, SIGNAL(clicked(bool)), m, SLOT(map()));
    m->setMapping(dotButton, ".");
    m->setMapping(delButton, "-");
    connect(m, SIGNAL(mapped(QString)), this, SLOT(softKeyInput(QString)));

    
    connect(OKJoinButton, SIGNAL(clicked(bool)), this, SLOT(beginJoinHost()));
    connect(cancelJoinButton, SIGNAL(clicked(bool)), this, SLOT(cancelJoinHost()));
    
    joinHostDialog->show();
}

void Dialog::softKeyInput(QString st) {
    qDebug() << st;
    if (st == "-") {
        int len = IPEdit->text().length();
        IPEdit->setText(IPEdit->text().left(len - 1));
        return;
    }
    IPEdit->setText(IPEdit->text().append(st));
}

void Dialog::waitForConnect() {
    //delete createHostDialog;
    OKWaitButton->setDisabled(true);
    if (listenSocket != nullptr) {
        delete listenSocket;
    }
    listenSocket = new QTcpServer;
    listenSocket->listen(QHostAddress(myIP), PORT);
    connect(listenSocket, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
}

void Dialog::acceptConnection() {
    delete createHostDialog;
    if (readWriteSocket != nullptr) {
        delete readWriteSocket;
    }
    readWriteSocket = listenSocket->nextPendingConnection();
    isHost = true;
    player = Map::Black;
    prepareToRead();
    //prepareGameWidget();
    //connect(readWriteSocket, SIGNAL(readyRead()), this, SLOT());
}

void Dialog::beginJoinHost() {
    QString IP = IPEdit->text();
    delete joinHostDialog;
    if (readWriteSocket != nullptr) {
        delete readWriteSocket;
    }
    readWriteSocket = new QTcpSocket();
    readWriteSocket->connectToHost(QHostAddress(IP), PORT);
    isHost = false;
    player = Map::White;
    connect(readWriteSocket, SIGNAL(connected()), this, SLOT(prepareToRead()));
    //connect(readWriteSocket, SIGNAL(readyRead()), this, SLOT());
}

void Dialog::cancelJoinHost() {
    delete joinHostDialog;
}

void Dialog::prepareToRead() {
    qDebug() << "Connected!";
    qDebug() << readWriteSocket->peerAddress();
    connect(readWriteSocket, SIGNAL(readyRead()), this, SLOT(calcRead()));
    sendUsername();
}

void Dialog::sendAvatar() {
    qDebug() << "Begin Send Avatar";
    QByteArray *array;
    array = new QByteArray();
    QDataStream ds(array, QIODevice::WriteOnly);
    ds << myAvatarPixmap;
    readWriteSocket->write(array->data());
    readWriteSocket->waitForBytesWritten();
    qDebug() << "Write Avatar OK";
}

void Dialog::sendUsername() {
    qDebug() << "Begin Send Username";
    QByteArray *array = new QByteArray();
    array->append(myAvatarFileName + "@" + myUserName);
    readWriteSocket->write(array->data());
    qDebug() << "Write Username OK";
}

void Dialog::prepareGameWidget() {          
    if (m_num != -1) {
        delete widget[m_num];
    }
    this->setFixedSize(GAME_WIDTH, GAME_HEIGHT);
    isGameWidget = true;

    QVBoxLayout *vt = new QVBoxLayout(this);
    QHBoxLayout *ht = new QHBoxLayout();
    QVBoxLayout *vt2 = new QVBoxLayout();
    QHBoxLayout *ht2 = new QHBoxLayout();
    vt->addLayout(ht);
    vt->addLayout(ht2);
    ht->addWidget(boardWidget);
    ht->addLayout(vt2);
    vt2->addWidget(myInfoWidget);
    vt2->addWidget(opInfoWidget);
    myNameLabel = new QLabel(myUserName, this);
    myAvatarLabel = new QLabel(this);
    myAvatarLabel->setPixmap(myAvatarPixmap.scaled(QSize(150, 150), Qt::KeepAspectRatio));
    opNameLabel = new QLabel(opUserName, this);
    opAvatarLabel = new QLabel(this);
    opAvatarLabel->setPixmap(opAvatarPixmap.scaled(QSize(150, 150), Qt::KeepAspectRatio));
    
    mySingleTimeLCD = new QLCDNumber(this);
    myTotalTimeLCD = new QLCDNumber(this);
    opSingleTimeLCD = new QLCDNumber(this);
    opTotalTimeLCD = new QLCDNumber(this);
    mySingleTimeLCD->display(20);
    opSingleTimeLCD->display(20);
    myTotalTimeLCD->display("00:00");
    opTotalTimeLCD->display("00:00");
    mySingleTimeLCD->setPalette(QPalette(Qt::black));
    opSingleTimeLCD->setPalette(QPalette(Qt::black));
    myTotalTimeLCD->setPalette(QPalette(Qt::black));
    opTotalTimeLCD->setPalette(QPalette(Qt::black));
    
    QVBoxLayout *vt3 = new QVBoxLayout(myInfoWidget);
    vt3->addWidget(myNameLabel, 0, Qt::AlignHCenter);
    vt3->addWidget(myAvatarLabel, 0, Qt::AlignHCenter);
    QGridLayout *gt = new QGridLayout();
    vt3->addLayout(gt);
    gt->addWidget(mySingleTimeLCD, 0, 1);
    gt->addWidget(myTotalTimeLCD, 1, 1);
    QLabel *mySingleLabel = new QLabel("单步倒计时", this);
    QLabel *myTotalLabel = new QLabel("总用时", this);
    gt->addWidget(mySingleLabel, 0, 0);
    gt->addWidget(myTotalLabel, 1, 0);
    
    QVBoxLayout *vt4 = new QVBoxLayout(opInfoWidget);
    vt4->addWidget(opNameLabel, 0, Qt::AlignHCenter);
    vt4->addWidget(opAvatarLabel, 0, Qt::AlignHCenter);
    QGridLayout *gt2 = new QGridLayout();
    vt4->addLayout(gt2);
    gt2->addWidget(opSingleTimeLCD, 0, 1);
    gt2->addWidget(opTotalTimeLCD, 1, 1);
    QLabel *opSingleLabel = new QLabel("单步倒计时", this);
    QLabel *opTotalLabel = new QLabel("总用时", this);
    gt2->addWidget(opSingleLabel, 0, 0);
    gt2->addWidget(opTotalLabel, 1, 0);
    
    withdrawButton = new QPushButton("悔棋", this);
    beginButton = new QPushButton("开始", this);
    quitButton = new QPushButton("退出", this);
    saveButton = new QPushButton("保存", this);
    loadButton = new QPushButton("载入", this);
    withdrawButton->setDisabled(true);
    
    ht2->addWidget(withdrawButton, 0, Qt::AlignHCenter);
    ht2->addWidget(beginButton, 0, Qt::AlignHCenter);
    ht2->addWidget(quitButton, 0, Qt::AlignHCenter);
    ht2->addWidget(saveButton, 0, Qt::AlignHCenter);
    ht2->addWidget(loadButton, 0, Qt::AlignHCenter);
    withdrawButton->setFocusPolicy(Qt::NoFocus);
    beginButton->setFocusPolicy(Qt::NoFocus);
    quitButton->setFocusPolicy(Qt::NoFocus);
    saveButton->setFocusPolicy(Qt::NoFocus);
    loadButton->setFocusPolicy(Qt::NoFocus);
    
    connect(quitButton, SIGNAL(clicked(bool)), this, SLOT(requestForQuit()));
    connect(saveButton, SIGNAL(clicked(bool)), this, SLOT(requestForSave()));
    connect(loadButton, SIGNAL(clicked(bool)), this, SLOT(requestForLoad()));
    connect(withdrawButton, SIGNAL(clicked(bool)), this, SLOT(requestForWithdraw()));
    connect(beginButton, SIGNAL(clicked(bool)), this, SLOT(prepareToBegin()));
 
    //isGameBegin = true;

    update();
}

void Dialog::requestForQuit() {
    sendMessage("requestQuit");
}

void Dialog::requestForLoad() {
    QString st = QFileDialog::getOpenFileName(this, "选取残局", "", "Text files (*.txt)");
    if (st == "")
        return;
    QStringList list = st.split("/");
    sendMessage("requestLoad#" + list[(int)list.size() - 1]);
}

void Dialog::requestForSave() {
    sendMessage("requestSave");
}

void Dialog::requestForWithdraw() {
    sendMessage("requestWithdraw");
}

void Dialog::prepareToBegin() {
    isMeBegin = true;
    sendMessage("start");
    beginButton->setDisabled(true);
    if (isOpBegin) {
        isGameBegin = true;
        connect(timer, SIGNAL(timeout()), this, SLOT(onTimerOut()));
        update();
        timer->start();
    }
}

void Dialog::changePlayer() {
    timer->stop();
    mySingleTime = opSingleTime = 20;
    mySingleTimeLCD->display(20);
    opSingleTimeLCD->display(20);
    if (map->curPlayer == Map::Black) {
        map->curPlayer = Map::White;
    }else {
        map->curPlayer = Map::Black;
    }
    update();
    timer->start();
}

void Dialog::onTimerOut() {
    if (map->curPlayer == player) {
        ++myTotalTime;
        QString myDisplayTime = "";
        if (myTotalTime / 60 < 10) {
            myDisplayTime += "0" + QString::number(myTotalTime / 60);
        }else {
            myDisplayTime += QString::number(myTotalTime / 60);
        }
        myDisplayTime += ":";
        if (myTotalTime % 60 < 10) {
            myDisplayTime += "0" + QString::number(myTotalTime % 60);
        }else {
            myDisplayTime += QString::number(myTotalTime % 60);
        }
        myTotalTimeLCD->display(myDisplayTime);
        if (--mySingleTime == 0) {
            mySingleTimeLCD->display(0);
            changePlayer();
            return;
        }
        mySingleTimeLCD->display(mySingleTime);
    }else {
        ++opTotalTime;
        QString opDisplayTime = "";
        if (opTotalTime / 60 < 10) {
            opDisplayTime += "0" + QString::number(opTotalTime / 60);
        }else {
            opDisplayTime += QString::number(opTotalTime / 60);
        }
        opDisplayTime += ":";
        if (opTotalTime % 60 < 10) {
            opDisplayTime += "0" + QString::number(opTotalTime % 60);
        }else {
            opDisplayTime += QString::number(opTotalTime % 60);
        }
        opTotalTimeLCD->display(opDisplayTime);
        if (--opSingleTime == 0) {
            opSingleTimeLCD->display(0);
            changePlayer();
            return;
        }
        opSingleTimeLCD->display(opSingleTime);
    }
}

void Dialog::renewTime() {
    timer->stop();
    mySingleTime = 20;
    mySingleTimeLCD->display(20);
    opSingleTime = 20;
    opSingleTimeLCD->display(20);
    update();
    timer->start();
}

bool Dialog::checkNoMyChess() {
    for (int i = 0; i < 15; ++i)
        for (int j = 0; j < 15; ++j)
            if (map->arr[i][j] == player)
                return false;
    return true;
}

void Dialog::InitialGame() {
    memset(map->arr, -1, sizeof map->arr);
    map->curPlayer = Map::Black;
    map->place.clear();
    
    mySingleTime = opSingleTime = 20;
    mySingleTimeLCD->display(20);
    opSingleTimeLCD->display(20);
    myTotalTimeLCD->display(0);
    opTotalTimeLCD->display(0);
    myTotalTime = opTotalTime = 0;
    
    isGameBegin = isOpBegin = isMeBegin = false;
    beginButton->setEnabled(true);
    withdrawButton->setDisabled(true);
    withDrawTime = 0;
    
    timer->stop();
    
    update();
}

void Dialog::calcRead() {
    /*if (opAvatarPixmap.isNull()) {
        opAvatarPixmap.loadFromData(readWriteSocket->readAll());
        sendUsername();
        return;
    }*/
    QString info;
    info = readWriteSocket->readAll();
    qDebug() << "calcRead " << info;
    if (opUserName == "") {
        QStringList infoList2 = info.split("@");
        opAvatarFileName = infoList2[0];
        opUserName = infoList2[1];
        opAvatarPixmap.load(opAvatarFileName);
        qDebug() << opAvatarFileName + " " + opUserName;
        prepareGameWidget();
        return;
    }
    if (info == "start") {
        isOpBegin = true;
        if (isMeBegin) {
            isGameBegin = true;
            connect(timer, SIGNAL(timeout()), this, SLOT(onTimerOut()));
            update();
            timer->start();
        }
        return;
    }
    if (info == "requestQuit") {
        QMessageBox::StandardButton b1 = QMessageBox::question(NULL, "Request", "对方请求退出游戏");
        if (b1 == QMessageBox::Yes) {
            sendMessage("agreeQuit");
            InitialGame();
            //this->close();
        }else {
            sendMessage("disagreeQuit");
        }
        return;
    }
    if (info == "agreeQuit") {
        InitialGame();
        //this->close();
        return;
    }
    if (info == "disagreeQuit") {
        QMessageBox::about(NULL, "Result", "对方不同意退出游戏");
        return;
    }
    if (info == "requestSave") {
        QMessageBox::StandardButton b2 = QMessageBox::question(NULL, "Request", "对方请求保存残局");
        if (b2 == QMessageBox::Yes) {
            QTime time = QTime::currentTime();
            map->save(QString::number(time.hour()) + "_" + QString::number(time.minute()) + ".txt");
            sendMessage("agreeSave");
        }else {
            sendMessage("disagreeSave");
        }
        return;
    }
    if (info == "agreeSave") {
        QTime time = QTime::currentTime();
        map->save(QString::number(time.hour()) + "_" + QString::number(time.minute()) + ".txt");
        return;
    }
    if (info == "disagreeSave") {
        QMessageBox::about(NULL, "Result", "对方不同意保存残局");
        return;
    }
    if (info.contains("requestLoad")) {
        QStringList list1 = info.split("#");
        QMessageBox::StandardButton b3 = QMessageBox::question(NULL, "Request", "对方请求加载残局" + list1[(int)list1.size() - 1]);
        if (b3 == QMessageBox::Yes) {
            map->load(list1[(int)list1.size() - 1]);
            update();
            sendMessage("agreeLoad#" + list1[(int)list1.size() - 1]);
        }else {
            sendMessage("disagreeLoad");
        }
        return;
    }
    if (info == "disagreeLoad") {
        QMessageBox::about(NULL, "Result", "对方不同意加载残局");
        return;
    }
    if (info.contains("agreeLoad")) {
        QStringList list2 = info.split("#");
        map->load(list2[(int)list2.size() - 1]);
        update();
        return;
    }
    if (info == "requestWithdraw") {
        QMessageBox::StandardButton b4 = QMessageBox::question(NULL, "Request", "对方请求悔棋");
        if (b4 == QMessageBox::Yes) {
            sendMessage("agreeWithdraw");
            map->pop(player ^ 1);
            renewTime();
        }else {
            sendMessage("disagreeWithdraw");
        }
        return;
    }
    if (info == "agreeWithdraw") {
        ++withDrawTime;
        if (withDrawTime >= 2) {
            withdrawButton->setDisabled(true);
        }
        map->pop(player);
        if (checkNoMyChess()) {
            withdrawButton->setDisabled(true);
        }
        renewTime();
        return;
    }
    if (info == "disagreeWithdraw") {
        QMessageBox::about(NULL, "Result", "对方不同意悔棋");
        return;
    }
    QStringList infoList = info.split("_");
    int x = infoList[0].toInt();
    int y = infoList[1].toInt();
    map->arr[x][y] = player ^ 1;
    map->place.push_back(QPoint(x, y));
    
    changePlayer();
    /*if (map->curPlayer == Map::Black)
        map->curPlayer = Map::White;
    else
        map->curPlayer = Map::Black;*/
    update();
    
    if (gameLogic->checkWin()) {
        gameOver(0);
    }
}

void Dialog::mousePressEvent(QMouseEvent *event) {
    if (!isGameBegin)
        return;
    if (player != map->curPlayer)
        return;
    int x = event->x(), y = event->y();
    int minDis = 100000000;
    int di = -1, dj = -1;
    for (int i = 0; i < 15; ++i)
        for (int j = 0; j < 15; ++j) {
            int curDis = (x - LCX - 40 * i) * (x - LCX - 40 * i) + (y - LCY - 40 * j) * (y - LCY - 40 * j);
            if (curDis < minDis) {
                minDis = curDis;
                di = i;
                dj = j;
            }
        }
    if (minDis > 400)
        return;
    if (map->arr[di][dj] != -1)
        return;
    putPoint(di, dj);
    calcSend(di, dj);
}

void Dialog::putPoint(int x, int y) {
    map->arr[x][y] = player;
    withDrawTime = 0;
    withdrawButton->setEnabled(true);
    map->place.push_back(QPoint(x, y));
    changePlayer();
    /*if (map->curPlayer == Map::Black)
        map->curPlayer = Map::White;
    else
        map->curPlayer = Map::Black;*/
    update();
    
    if (gameLogic->checkWin()) {
        gameOver(1);
    }
}

void Dialog::sendMessage(QString st) {
    QByteArray *array = new QByteArray;
    array->clear();
    array->append(st);
    readWriteSocket->write(array->data());
}

void Dialog::calcSend(int x, int y) {
    this->sendMessage(QString::number(x) + "_" + QString::number(y));
    /*QByteArray *array = new QByteArray;
    array->clear();
    array->append(QString::number(x) + "_" + QString::number(y));
    readWriteSocket->write(array->data());*/
}

void DrawPoint(QPainter &painter, int x, int y, int size, QColor color = Qt::black) {
    int nx = LCX + x * 40, ny = LCY + y * 40;
    painter.setPen(color);
    painter.setBrush(color);
    painter.drawEllipse(nx - size / 2, ny - size / 2, size, size);
}

void Dialog::paintEvent(QPaintEvent *event) {
    if (!isGameWidget) {
        return;
    }
    

    QPainter painter(this);
    
    painter.drawPixmap(0, 0, width(), height(), background);
    QColor color = QColor(200, 124, 30, 128);
    painter.setPen(color);
    painter.setBrush(color);
    painter.drawRect(0, 0, 600, 600);
    for (int i = 0; i < 15; ++i) {
        painter.setPen(Qt::black);
        painter.drawLine(LCX + i * 40, LCY, LCX + i * 40, LCY + 560);
        painter.drawLine(LCX, LCY + i * 40, LCX + 560, LCY + i * 40);
    }
    DrawPoint(painter, 3, 3, 8);
    DrawPoint(painter, 3, 11, 8);
    DrawPoint(painter, 11, 3, 8);
    DrawPoint(painter, 11, 11, 8);
    DrawPoint(painter, 7, 7, 8);
    
    for (int i = 0; i < 15; ++i)
        for (int j = 0; j < 15; ++j) {
            if (map->arr[i][j] == -1)
                continue;
            if (map->arr[i][j] == Map::Black)
                DrawPoint(painter, i, j, 24, Qt::black);
            else
                DrawPoint(painter, i, j, 24, Qt::white);
        }
    
    if (isGameBegin) {
        painter.setPen(Qt::blue);
        painter.setBrush(Qt::NoBrush);
        if (map->curPlayer == player) {
            painter.drawRect(myInfoWidget->geometry());
        }else {
            painter.drawRect(opInfoWidget->geometry());
        }
    }
}

void Dialog::gameOver(bool isWin) {
    //disconnect(readWriteSocket, SIGNAL(readyRead()), this, SLOT(calcRead()));
    disconnect(timer, SIGNAL(timeout()), this, SLOT(onTimerOut()));
    timer->stop();
    
    
    if (isWin) {
        QSound::play(":/sound/sound/win.wav");
    }else {
        QSound::play(":/sound/sound/lose.wav");
    }
    gameOverDialog = new QDialog();
    gameOverDialog->setWindowTitle("Game Over");
    gameOverDialog->setFixedSize(300, 200);
    QVBoxLayout *vt = new QVBoxLayout(gameOverDialog);
    QLabel *label = new QLabel(isWin ? "你赢了!" : "你输了!", gameOverDialog);
    QLabel *myTimeLabel = new QLabel(gameOverDialog);
    myTimeLabel->setText("你的总用时为" + QString::number(myTotalTime / 60) + "分" + QString::number(myTotalTime % 60) + "秒");
    QLabel *opTimeLabel = new QLabel(gameOverDialog);
    opTimeLabel->setText("对方的总用时为" + QString::number(opTotalTime / 60) + "分" + QString::number(opTotalTime % 60) + "秒");
    OKGameOverButton = new QPushButton("确定", gameOverDialog);
    vt->addWidget(label, 0, Qt::AlignHCenter);
    vt->addWidget(myTimeLabel, 0, Qt::AlignHCenter);
    vt->addWidget(opTimeLabel, 0, Qt::AlignHCenter);
    vt->addWidget(OKGameOverButton, 0, Qt::AlignHCenter);
    connect(OKGameOverButton, SIGNAL(clicked(bool)), gameOverDialog, SLOT(close()));
    gameOverDialog->show();
}

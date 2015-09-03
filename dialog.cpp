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
    map = new Map();
    timer = new QTimer();
    timer->setInterval(1000);
    gameLogic = new GameLogic(map);
    
    boardWidget = new QWidget(this);
    boardWidget->setFixedSize(600, 600);
    myInfoWidget = new QWidget();
    myInfoWidget->setFixedWidth(200);
    opInfoWidget = new QWidget();
    opInfoWidget->setFixedWidth(200);
    
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimerOut()));
    
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
    chooseFightModelWidget();
}

void Dialog::chooseAvatarFromFile() {
    QString st = QFileDialog::getOpenFileName(this, "选取头像", "", "Image Files(*.jpg *.jpeg)");
    QPixmap pixmap(st);
    QSize size(chooseAvatarLabel->width(), chooseAvatarLabel->height());
    QPixmap pixmap2 = pixmap.scaled(size, Qt::KeepAspectRatio);
    if (!pixmap2.isNull()) {
        myAvatarFileName = st;
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
    OKWaitButton = new QPushButton("确定", createHostDialog);
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
    delete createHostDialog;
}

void Dialog::joinHost() {
    joinHostDialog = new QDialog();
    joinHostDialog->setWindowTitle("准备加入主机");
    joinHostDialog->setFixedSize(400, 400);
    
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
    delButton = new QPushButton("Backspace", joinHostDialog);
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
    delete createHostDialog;
    if (listenSocket != nullptr) {
        delete listenSocket;
    }
    listenSocket = new QTcpServer;
    listenSocket->listen(QHostAddress(myIP), PORT);
    connect(listenSocket, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
}

void Dialog::acceptConnection() {
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
    
    connect(beginButton, SIGNAL(clicked(bool)), this, SLOT(prepareToBegin()));
 
    //isGameBegin = true;

    update();
}

void Dialog::prepareToBegin() {
    isMeBegin = true;
    sendMessage("start");
    beginButton->setDown(true);
    if (isOpBegin) {
        isGameBegin = true;
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

void Dialog::calcRead() {
    qDebug() << "calcRead";
    /*if (opAvatarPixmap.isNull()) {
        opAvatarPixmap.loadFromData(readWriteSocket->readAll());
        sendUsername();
        return;
    }*/
    QString info;
    info = readWriteSocket->readAll();
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
            timer->start();
        }
        return;
    }
    QStringList infoList = info.split("_");
    int x = infoList[0].toInt();
    int y = infoList[1].toInt();
    map->arr[x][y] = player ^ 1;
    
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
    
    putPoint(di, dj);
    calcSend(di, dj);
}

void Dialog::putPoint(int x, int y) {
    map->arr[x][y] = player;
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
    painter.setPen(Qt::yellow);
    painter.setBrush(Qt::yellow);
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

}

void Dialog::gameOver(bool isWin) {
    disconnect(readWriteSocket, SIGNAL(readyRead()), this, SLOT(calcRead()));
    
    gameOverDialog = new QDialog();
    gameOverDialog->setWindowTitle("Game Over");
    gameOverDialog->setFixedSize(300, 200);
    QVBoxLayout *vt = new QVBoxLayout(gameOverDialog);
    QLabel *label = new QLabel(isWin ? "你赢了!" : "你输了!", gameOverDialog);
    OKGameOverButton = new QPushButton("确定", gameOverDialog);
    vt->addWidget(label, 0, Qt::AlignHCenter);
    vt->addWidget(OKGameOverButton, 0, Qt::AlignHCenter);
    connect(OKGameOverButton, SIGNAL(clicked(bool)), gameOverDialog, SLOT(close()));
    gameOverDialog->show();
}

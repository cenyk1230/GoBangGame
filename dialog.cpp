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

const static int INIT_WIDTH = 200;
const static int INIT_HEIGHT = 200;
const static int GAME_WIDTH = 600;//780;
const static int GAME_HEIGHT = 600;//650;
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
    listenSocket = nullptr;
    readWriteSocket = nullptr;
    map = new Map();
    
    //test
    //prepareGameWidget();
    
    chooseFightModelWidget();
}

void Dialog::chooseFightModelWidget() {
    if (m_num != -1) {
        delete widget[m_num];
    }
    m_num = 0;
    widget[m_num] = new QWidget(this);
    widget[m_num]->setFixedSize(INIT_WIDTH, INIT_HEIGHT);
    networkFightButton = new QPushButton("网络对战", widget[m_num]);
    personAIFightButton = new QPushButton("人机对战", widget[m_num]);
    QVBoxLayout *vt = new QVBoxLayout(widget[m_num]);
    vt->addWidget(networkFightButton, 0, Qt::AlignHCenter);
    vt->addWidget(personAIFightButton, 0, Qt::AlignHCenter);
    connect(networkFightButton, SIGNAL(clicked(bool)), this, SLOT(networkGameWidget()));
}

void Dialog::networkGameWidget() {
    if (m_num != -1) {
        delete widget[m_num];
    }
    m_num = 1;
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
            if (list[0] == "127" || list[0] == "10")
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
    prepareGameWidget();
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
    connect(readWriteSocket, SIGNAL(connected()), this, SLOT(prepareGameWidget()));
    //connect(readWriteSocket, SIGNAL(readyRead()), this, SLOT());
}

void Dialog::cancelJoinHost() {
    delete joinHostDialog;
}

void Dialog::prepareGameWidget() {
    qDebug() << "Connected!";
    if (m_num != -1) {
        delete widget[m_num];
    }
    this->setFixedSize(GAME_WIDTH, GAME_HEIGHT);
    isGameWidget = true;
    
    //TODO ...
    isGameBegin = true;
    
    connect(readWriteSocket, SIGNAL(readyRead()), this, SLOT(calcRead()));
    
    update();
}

void Dialog::calcRead() {
    QString info;
    info = readWriteSocket->readAll();
    QStringList infoList = info.split("_");
    int x = infoList[0].toInt();
    int y = infoList[1].toInt();
    map->arr[x][y] = player ^ 1;
    if (map->curPlayer == Map::Black)
        map->curPlayer = Map::White;
    else
        map->curPlayer = Map::Black;
    update();
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
    if (map->curPlayer == Map::Black)
        map->curPlayer = Map::White;
    else
        map->curPlayer = Map::Black;
    update();
}

void Dialog::calcSend(int x, int y) {   
    QByteArray *array = new QByteArray;
    array->clear();
    array->append(QString::number(x) + "_" + QString::number(y));
    readWriteSocket->write(array->data());
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

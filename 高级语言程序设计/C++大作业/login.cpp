#include "login.h"
#include "ui_login.h"
#include <introduction.h>

Login::Login(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Login)
{
    ui->setupUi(this);
    this->setGeometry(QRect(600,300,480,480));
    QPixmap pixmap(":/myImage/images/sky.png");
    QPalette palette;
    palette.setBrush(backgroundRole(), QBrush(pixmap));
    setPalette(palette);
    this->setWindowTitle("开始游戏");
}

Login::~Login()
{
    delete ui;
}

void Login::on_pushButton_clicked()
{
    this->close();
    f=new Bird();
    f->show();
}


void Login::on_pushButton_2_clicked()
{
    Introduction* intro=new Introduction();
    this->close();
    intro->show();
}

#include "introduction.h"
#include "ui_introduction.h"
#include <bird.h>

Introduction::Introduction(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Introduction)
{

    ui->setupUi(this);
    this->setGeometry(QRect(600,300,480,480));
    QPixmap pixmap(":/myImage/images/sky.png");
    QPalette palette;
    palette.setBrush(backgroundRole(), QBrush(pixmap));
    setPalette(palette);
    ui->text->setStyleSheet("background:transparent;border-width:0;border-style:outset");
    this->setWindowTitle("游戏介绍");
}

Introduction::~Introduction()
{
    delete ui;
}

void Introduction::on_pushButton_clicked()
{
    Bird* bird=new Bird();
    this->close();
    bird->show();
}

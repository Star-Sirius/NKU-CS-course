#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include <bird.h>


namespace Ui {
class Login;

}

class Login : public QWidget
{
    Q_OBJECT

public:

    explicit Login(QWidget *parent = nullptr);
    ~Login();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::Login *ui;
    Bird *f;
};

#endif // LOGIN_H

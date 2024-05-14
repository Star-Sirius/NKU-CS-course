#ifndef END_H
#define END_H

#include <QWidget>

namespace Ui {
class End;
}

class End : public QWidget
{
    Q_OBJECT

public:
    explicit End(QWidget *parent = nullptr);
    ~End();
    void manage(int a,int b);
    static int maxijump;
    int thisjump;
private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();

private:
    Ui::End *ui;

};

#endif // END_H

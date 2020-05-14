#ifndef NAMEREQUEST_H
#define NAMEREQUEST_H

#include <QDialog>
#include <QListWidget>

namespace Ui {
class nameRequest;
}

class nameRequest : public QDialog
{
    Q_OBJECT

public:
    explicit nameRequest(QListWidget* __list, QWidget *parent = nullptr);
    ~nameRequest();

signals:
    void transmit(QString name);

private slots:
    void on_buttonBox_accepted();

private:
    Ui::nameRequest *ui;
    QListWidget* list;
};

#endif // NAMEREQUEST_H

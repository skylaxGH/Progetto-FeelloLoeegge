#ifndef SCELTAPLAYLIST_H
#define SCELTAPLAYLIST_H

#include <QDialog>
#include <QListWidget>

namespace Ui {
class sceltaPlaylist;
}

class sceltaPlaylist : public QDialog
{
    Q_OBJECT

public:
    explicit sceltaPlaylist(QListWidget* __list, QWidget *parent = nullptr);
    ~sceltaPlaylist();

signals:
    void transmit(QString name);

private slots:
    void on_btnAggiorna_clicked();

    void on_listaPlaylist_itemClicked(QListWidgetItem *item);

private:
    Ui::sceltaPlaylist *ui;
    QListWidget* list;
};

#endif // SCELTAPLAYLIST_H

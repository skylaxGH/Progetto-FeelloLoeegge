#include "sceltaplaylist.h"
#include "ui_sceltaplaylist.h"

#include <QMessageBox>

sceltaPlaylist::sceltaPlaylist(QListWidget* __list, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::sceltaPlaylist),
    list(__list)
{
    ui->setupUi(this);
}

sceltaPlaylist::~sceltaPlaylist()
{
    delete ui;
}

void sceltaPlaylist::on_btnAggiorna_clicked() {
    ui->listaPlaylist->clear();
    for(int i = 0; i < list->count(); i++) {
        ui->listaPlaylist->addItem(list->item(i)->text());
    }
}

void sceltaPlaylist::on_listaPlaylist_itemClicked(QListWidgetItem *item)
{
    emit transmit(item->text());
    close();
}

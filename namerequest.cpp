#include "namerequest.h"
#include "ui_namerequest.h"
#include <QMessageBox>

nameRequest::nameRequest(QListWidget* __list, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::nameRequest),
    list(__list)
{
    ui->setupUi(this);
}

nameRequest::~nameRequest()
{
    delete ui;
}

void nameRequest::on_buttonBox_accepted()
{
    bool ctr = false;

    for(int i = 0; i < list->count(); i++) {
        if(ui->namePlaylist->text() == list->item(i)->text()) {
            ctr = true;
        }
    }

    if(ctr == true || ui->namePlaylist->text() == "" || ui->namePlaylist->text().contains('.') ||
       ui->namePlaylist->text().contains('\\') || ui->namePlaylist->text().contains('/') ||
       ui->namePlaylist->text().contains(':') || ui->namePlaylist->text().contains('*') ||
       ui->namePlaylist->text().contains('|') || ui->namePlaylist->text().contains('?') ||
       ui->namePlaylist->text().contains('<') || ui->namePlaylist->text().contains('>') ||
       ui->namePlaylist->text().contains('"')
      ) {
        QMessageBox error;
        error.setWindowTitle("Errore");
        error.setText("Hai inserito un nome di playlist errato, gia' esistente oppure vuoto");
        error.exec();
    }

    else emit transmit(ui->namePlaylist->text());

    ui->namePlaylist->clear();
}

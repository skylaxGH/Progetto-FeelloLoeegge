#include "namerequest.h"
#include "ui_namerequest.h"

#include <QMessageBox>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

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

    QFile filePlaylist(PROJECT_PATH + playlistFile);
    filePlaylist.open(QIODevice::ReadWrite);
    QTextStream stream(&filePlaylist);

    while (!stream.atEnd()) {
        QString line = stream.readLine();

        QFile playlistPath(line);
        QFileInfo playlistInfo(playlistPath.fileName());
        QString playlistName(playlistInfo.fileName());

        if(ui->namePlaylist->text() == playlistName) ctr = true;
    }

    filePlaylist.close();

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

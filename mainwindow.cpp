#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QPushButton>
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    nRequest = new nameRequest(ui->listaPlaylist, nullptr);
    connect(nRequest, &nameRequest::transmit, this, &MainWindow::updateName);

    player->setMedia(nullptr);

    playlistFile = PROJECT_PATH + playlistFile;
    QFile filePlaylist(playlistFile);
    filePlaylist.open(QIODevice::WriteOnly);
    filePlaylist.close();

    this->setFixedSize(this->size());  //La grandezza della finestra non può essere modificata (dimensione fissa)

    ui->volumeSlider->setVisible(false);                            //Lo slider del volume di default non è visibile
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::selezioneFile() {
    QDir pathDir;  //La variabile pathDir serve per l'utilizzo della funzione toNativeSeparators

    path = QFileDialog::getOpenFileName(this, "Apri un file", "C://", "*.mp3 *.mp4 *.wav *.wma *.aac *.m4a");  //Apro una finestra di comunicazione con
                                                                                                               //l'utente per l'apertura del file
    path = pathDir.toNativeSeparators(path);   //Converto la path Windows ottenuta per la traccia audio in una path Unix
                                               //per lavorare con QT

    QFile songPath(path);                      //Prendo la path della traccia audio in una QFile per usare filename()
    QFileInfo songInfo(songPath.fileName());   //Salvo in una QFileInfo il filename di songpath (quindi solo il nome della canzone)
    QString songName(songInfo.fileName());     //Salvo il nome ottenuto in una string per mostrarla all'utente

    ui->songName->setText(songName);

    player->setMedia(QUrl::fromLocalFile(path));  //Imposto come file per la riproduzione il file scelto da utente per indirizzo

    player->setVolume(ui->volumeSlider->value()); //Imposto il volume di base al valore del volumeSlider
    player->play();                               //Faccio partire la riproduzione

    /* Connetto tutti gli slot di segnale tra lo stream dell'audio e i vari slider/progress bar
       per aumentare/diminuire posizione/volume */

    connect(player, &QMediaPlayer::durationChanged, this, &MainWindow::on_durationChanged);
    connect(ui->volumeSlider, SIGNAL(valueChanged(int)), player, SLOT(setVolume(int)));   //Il valore int ritornato da valueChanged del volumeSlider sara il valore int dato come parametro a setVolume del player
    connect(player, &QMediaPlayer::positionChanged, this, &MainWindow::on_positionChanged);
}



void MainWindow::on_btnRiproduci_clicked() {
    QMessageBox box;

    if(path == "") {
        box.setWindowTitle("Errore");
        box.setText("Non hai selezionato nessuna traccia audio, vuoi selezionarne una?");
        box.addButton("Apri", QMessageBox::AcceptRole);
        box.addButton(QMessageBox::No);
        int ret = box.exec();

        if(ret == QMessageBox::AcceptRole) selezioneFile();
    }

    /*else if(path.length() > 0) {
        QMessageBox::StandardButton answer = QMessageBox::question(this, "Avviso", "C'è già un file in riproduzione, vuoi selezionarne un'altro?",
                                                                   QMessageBox::Yes | QMessageBox::No);
        if(answer == QMessageBox::Yes) {
            player->stop();
            selezioneFile();
        }
    }*/

    else if(player->state() == QMediaPlayer::PausedState) player->play();
}




void MainWindow::on_btnPausa_clicked() {
    QMessageBox box;

    if(player->isAudioAvailable() == false) {
        box.setWindowTitle("Errore");
        box.setText("Non c'è nessuna canzone in riproduzione");
        box.exec();
    }

    else player->pause();
}

void MainWindow::on_btnStop_clicked() {
    QMessageBox box;

    if(player->isAudioAvailable() == false) {  //In caso il file audio non sia disponibile per la riproduzione..
        box.setWindowTitle("Errore");
        box.setText("Non c'è nessuna canzone in riproduzione");
        box.exec(); //Messaggio di errore
    }

    else {
        player->stop();             //Negli altri casi stoppo la riproduzione, do valore nullptr a setMedia per segnalare che non è disponibile
        player->setMedia(nullptr);  //nessun file audio

        path = "";
        ui->songName->clear();      //Pulisco il file songName così posso prendere in input il nome del file di un'altra canzone
        ui->timeLabel->setText("0:00");


        disconnect(ui->volumeSlider, SIGNAL(valueChanged(int)), player, SLOT(setVolume(int)));     //Disconnetto i vari slot di segnale tra stream audio e slider/progress bar
        disconnect(player, &QMediaPlayer::positionChanged, this, &MainWindow::on_positionChanged);
        disconnect(player, &QMediaPlayer::durationChanged, this, &MainWindow::on_durationChanged);
    }
}

void MainWindow::on_btnVolume_clicked() {
    if(slider == true) ui->volumeSlider->setVisible(false);
    else {
        ui->volumeSlider->setVisible(true);
    }

    slider = !slider;
}

void MainWindow::on_progressSlider_sliderMoved(int position) {
    if(player->isAudioAvailable() == false) {}
    else player->setPosition(position);
}

void MainWindow::on_positionChanged(qint64 position) {
    double sec;

    ui->progressSlider->setValue(position);

    sec = static_cast<double>(position) / 1000;

    sec++;


        if (int (sec) - (min * 60) == 60) {

            min++;

            ui->timeLabel->setText(QString::number(static_cast<int>(min)) + ":" + QString::number(static_cast<int>(sec) - (min * 60)));
        }

        else {

            if(int (sec) - (min * 60) < 0){

                min--;
                ui->timeLabel->setText(QString::number(static_cast<int>(min)) + ":" + QString::number(static_cast<int>(sec) - (min * 60)));
            }

            ui->timeLabel->setText(QString::number(static_cast<int>(min)) + ":" + QString::number(static_cast<int>(sec) - (min * 60)));
        }
}

void MainWindow::on_durationChanged(qint64 position) {
    double min = 0, sec = 0;

    ui->progressSlider->setMaximum(position);

    sec = static_cast<double>(position) / 1000;
    if(int(sec) % 60 == 0) {
        min = sec / 60;
        ui->durationLabel->setText(QString::number(min) + ":00");
    }

    else {
        min = sec / 60;
        sec = (int(sec) % 60) + 1;
        ui->durationLabel->setText(QString::number(static_cast<int>(min)) + ":" + QString::number(sec));
    }
}

void MainWindow::on_btnCreaplaylist_clicked()
{
    nRequest->setModal(true);   //Imposto la finestra parente non cliccabile finchè la nuova finestra non finisce il suo corso
    nRequest->show();
}

void MainWindow::updateName(QString name) {
    ui->listaPlaylist->addItem(name);   //Aggiungo la playlist alla lista

    QDir dir(PROJECT_PATH + name);      //PROJECT_PATH costante che contiene la path del progetto (guardare .pro)
    if (!dir.exists()) dir.mkpath("."); //Se la cartella non esiste, la creo (. si riferisce alla path attualmente puntata da dir)

    QString plDir = PROJECT_PATH + name;
    QFile filePlaylist(playlistFile);
    filePlaylist.open(QIODevice::Append);

    QTextStream stream(&filePlaylist);
    stream << plDir << endl;

    filePlaylist.close();

    QString mod = "/" + name + ".txt";  //Per costruire la path assoluta per il file di testo aggiungo il nome del file più lo slash ed il .txt

    QString playlistName = PROJECT_PATH + name + mod;
    QFile textPlaylist(playlistName);
    textPlaylist.open(QIODevice::WriteOnly);
    textPlaylist.close();
}

void MainWindow::on_btnAggiungip_clicked()
{
    if(ui->listaPlaylist->currentRow() - 1 || ui->listaPlaylist->count() == 0) {
        QMessageBox box;
        box.setWindowTitle("Errore");
        box.setText("Non hai selezionato nessuna playlist o non ne hai creata una");
        box.exec();
    }

    else {

    }
}



//----------------AZIONI--------------------

void MainWindow::on_actionApri_un_file_triggered() { //Imposto tutte le azioni della Menu Bar collegandole alle funzioni già create
    MainWindow::on_btnRiproduci_clicked();
}

void MainWindow::on_actionStart_triggered() {
    MainWindow::on_btnRiproduci_clicked();
}

void MainWindow::on_actionStop_triggered() {
    MainWindow::on_btnStop_clicked();
}

void MainWindow::on_actionPausa_triggered() {
    MainWindow::on_btnPausa_clicked();
}

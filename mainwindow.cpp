#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    nRequest = new nameRequest(ui->listaPlaylist, nullptr);
    connect(nRequest, &nameRequest::transmit, this, &MainWindow::updateName);

    sPlaylist = new sceltaPlaylist(ui->listaPlaylist, nullptr);
    connect(sPlaylist, &sceltaPlaylist::transmit, this, &MainWindow::insertSong);

    player->setMedia(nullptr);

    ui->songName->setText("CODA DI RIPRODUZIONE");

    playlistFile = PROJECT_PATH + playlistFile;

    QFile filePlaylist(playlistFile);
    filePlaylist.open(QIODevice::ReadWrite);
    QTextStream stream(&filePlaylist);

    while (!stream.atEnd()) {
        QString line = stream.readLine();

        QFile playlistPath(line);
        QFileInfo playlistInfo(playlistPath.fileName());
        QString playlistName(playlistInfo.fileName());

        ui->listaPlaylist->addItem(playlistName);
    }

    filePlaylist.close();

    this->setFixedSize(this->size());  //La grandezza della finestra non pu√≤ essere modificata (dimensione fissa)

    ui->volumeSlider->setVisible(false);  //Lo slider del volume di default non √® visibile
}

MainWindow::~MainWindow()
{
    delete ui;
}

/*-------------------------------GESTIONE RIPRODUZIONE TRACCE------------------------------*/

void MainWindow::selezioneFile(int n) {
    QDir pathDir;  //La variabile pathDir serve per l'utilizzo della funzione toNativeSeparators

    path = QFileDialog::getOpenFileName(this, "Apri un file", "C://", "*.mp3 *.wav");  //Apro una finestra di comunicazione con
                                                                                       //l'utente per l'apertura del file
    if(path != "") {
        path = pathDir.toNativeSeparators(path);   //Converto la path Windows ottenuta per la traccia audio in una path Unix
                                                   //per lavorare con QT

        QFile songPath(path);                      //Prendo la path della traccia audio in una QFile per usare filename()
        QFileInfo songInfo(songPath.fileName());   //Salvo in una QFileInfo il filename di songpath (quindi solo il nome della canzone)
        QString songName(songInfo.completeBaseName());  //Salvo il nome ottenuto in una string per mostrarla all'utente

        ui->listaTitolo->addItem(songName);

        if(n == 0) {
            coda->addMedia(QUrl::fromLocalFile(path));
            coda->setCurrentIndex(1);

            player->setPlaylist(coda);

            player->setVolume(ui->volumeSlider->value()); //Imposto il volume di base al valore del volumeSlider
            player->play();                               //Faccio partire la riproduzione

            QListWidgetItem* item = ui->listaTitolo->item(0);
            ui->listaTitolo->setSelectionMode(QAbstractItemView::SingleSelection);
            item->setSelected(true);
            ui->listaTitolo->setSelectionMode(QAbstractItemView::NoSelection);

            /* Connetto tutti gli slot di segnale tra lo stream dell'audio e i vari slider/progress bar
               per aumentare/diminuire posizione/volume */

            connect(ui->volumeSlider, SIGNAL(valueChanged(int)), player, SLOT(setVolume(int)));   //Il valore int ritornato da valueChanged del volumeSlider sara il valore int dato come parametro a setVolume del player
            connect(player, &QMediaPlayer::durationChanged, this, &MainWindow::on_durationChanged);
            connect(player, &QMediaPlayer::positionChanged, this, &MainWindow::on_positionChanged);
            connect(player, &QMediaPlayer::mediaStatusChanged, this, &MainWindow::updateCoda);
        }

        else coda->addMedia(QUrl::fromLocalFile(path));
    }
}


void MainWindow::updateCoda() {
    if(player->state() == QMediaPlayer::StoppedState && coda->mediaCount() > 1) {
        coda->setCurrentIndex(coda->currentIndex() + 1);
        player->play();
    }

    else if(coda->mediaCount() == 0) {
        if(player->state() == QMediaPlayer::StoppedState) player->setMedia(nullptr);
    }
}

/*-------------------------------GESTIONE PULSANTI/SLIDER------------------------------*/

void MainWindow::on_btnRiproduci_clicked() {
    QMessageBox box;

    if(path == "" || !player->isAudioAvailable()) {
        box.setWindowTitle("Errore");
        box.setText("Non hai selezionato nessuna traccia audio, vuoi selezionarne una?");
        box.addButton("Apri", QMessageBox::AcceptRole);
        box.addButton(QMessageBox::No);
        int ret = box.exec();

        if(ret == QMessageBox::AcceptRole) selezioneFile(0);
    }

    else if(player->state() == QMediaPlayer::PausedState) player->play();

    if(player->isAudioAvailable()){

            box.setWindowTitle("Canzone gi‡ in riproduzione");
            box.setText("C'Ë una canzone gi‡ in riproduzione, vuoi metterne un altra in coda?");
            box.addButton("Apri", QMessageBox::AcceptRole);
            box.addButton(QMessageBox::No);
            int ret = box.exec();

            if(ret == QMessageBox::AcceptRole) selezioneFile(1);
        }
}

void MainWindow::on_btnPausa_clicked() {
    QMessageBox box;

    if(player->isAudioAvailable() == false) {
        box.setWindowTitle("Errore");
        box.setText("Non c'Ë nessuna canzone in riproduzione");
        box.exec();
    }

    else player->pause();
}

void MainWindow::on_btnStop_clicked() {
    QMessageBox box;

    if(player->isAudioAvailable() == false) {  //In caso il file audio non sia disponibile per la riproduzione..
        box.setWindowTitle("Errore");
        box.setText("Non c'Ë nessuna canzone in riproduzione");
        box.exec(); //Messaggio di errore
    }

    else {
        player->stop();             //Negli altri casi stoppo la riproduzione, do valore nullptr a setMedia per segnalare che non √® disponibile
        player->setMedia(nullptr);  //nessun file audio
        coda->clear();

        path = "";
        ui->timeLabel->setText("0:00");

        ui->listaTitolo->clear();

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

/*-------------------------------GESTIONE TEMPO DI RIPRODUZIONE/DURATA TOTALE------------------------------*/

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

/*-------------------------------GESTIONE CODA/PLAYLIST (file di testo)------------------------------*/

void MainWindow::on_btnCreaplaylist_clicked() {
    nRequest->setModal(true);   //Imposto la finestra parente non cliccabile finch√® la nuova finestra non finisce il suo corso
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

    QString mod = "/" + name + ".txt";  //Per costruire la path assoluta per il file di testo aggiungo il nome del file pi√π lo slash ed il .txt

    QString playlistName = PROJECT_PATH + name + mod;
    QFile textPlaylist(playlistName);
    textPlaylist.open(QIODevice::WriteOnly);
    textPlaylist.close();
}

void MainWindow::on_btnAggiungip_clicked() {
    sPlaylist->setModal(true);
    sPlaylist->show();
}

void MainWindow::insertSong(QString name) {
    if(path != "") {
        QString playlistPath, samestr = name;
        playlistPath = PROJECT_PATH + samestr;

        QFile songFile(path);
        QFileInfo songInfo(songFile.fileName());
        QString songName(songInfo.fileName());

        QString playlistText = playlistPath + "/" + samestr + ".txt";

        QMessageBox box;
        QFile songText(playlistText);
        bool ctr = false;

        if(songText.open(QIODevice::ReadOnly)) {
            QTextStream stream(&songText);
            while (!stream.atEnd()) {
                 QString line = stream.readLine();
                 if(playlistText + "/" + songName == line) ctr = true;
            }
            songText.close();
        }

        if(ctr == true) {
            box.setWindowTitle("Errore");
            box.setText("Canzone gi‡ presente nella playlist");
            box.exec();
        }

        else {
            playlistPath = playlistPath + "/" + songName;

            QFile songPath(playlistText);
            songPath.open(QIODevice::Append);
            QTextStream stream(&songPath);
            stream << playlistPath << endl;
            songPath.close();

            songFile.rename(playlistPath);

            box.setWindowTitle("Aggiunta correttamente");
            box.setText("Canzone aggiunta correttamente alla playlist");
            box.exec();
        }
    }
}

void MainWindow::on_listaPlaylist_itemClicked(QListWidgetItem *item) {
    ui->songName->setText(item->text());
    ui->listaTitolo->clear();

    if(player->isAudioAvailable()) {
        player->stop();
        player->setMedia(nullptr);
    }

    QString playlistxtPath, samestr = item->text();
    playlistxtPath = PROJECT_PATH + samestr + "/" + samestr + ".txt";

    QFile songPaths(playlistxtPath);

    if (songPaths.open(QIODevice::ReadOnly)) {

        QTextStream stream(&songPaths);
        while (!stream.atEnd()) {
            QString line = stream.readLine();
            playlist->addMedia(QUrl::fromLocalFile(line));

            QFile songLine(line);
            QFileInfo songInfo(songLine.fileName());
            QString songName(songInfo.completeBaseName());

            ui->listaTitolo->addItem(songName);
        }
        songPaths.close();
    }

    playlist->setCurrentIndex(1);
    player->setPlaylist(playlist);
    player->play();

    connect(ui->volumeSlider, SIGNAL(valueChanged(int)), player, SLOT(setVolume(int)));
    connect(player, &QMediaPlayer::durationChanged, this, &MainWindow::on_durationChanged);
    connect(player, &QMediaPlayer::positionChanged, this, &MainWindow::on_positionChanged);
}



//----------------AZIONI--------------------

void MainWindow::on_actionApri_un_file_triggered() { //Imposto tutte le azioni della Menu Bar collegandole alle funzioni gi√  create
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

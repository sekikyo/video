#include "mainwindow.h"
#include "global_value.h"

MainWindow::MainWindow(QWidget *parent) :BaseWindow(parent),mediaHasUpdate(false)
{
    initData();
    initLayout();
    initConnection();
    slot_updateMedia();
}

void MainWindow::initData()
{
    // Initialize global main class of 'MainWindow' for other widgets invokes.
    mainWindow = this;
    // Start media source update thread.
    // Uevent for usb and inotify for file modify.
    ueventThread = new UeventThread(this);
    ueventThread->start();
    inotifyThread = new InotifyThread(this);
    inotifyThread->start();
}

void MainWindow::initLayout(){
    QVBoxLayout *mainLayout = new QVBoxLayout;

    m_videoWid = new VideoWidgets(this);
    mainLayout->addWidget(m_videoWid);
    mainLayout->setContentsMargins(0,0,0,0);

    setLayout(mainLayout);
}

void MainWindow::initConnection()
{
    // Update media resource when receive signals from 'uevent' or 'inotify'.
    connect(this,SIGNAL(beginUpdateMediaResource()),this,SLOT(slot_setUpdateFlag()));
    connect(this,SIGNAL(updateUiByRes(QFileInfoList)),this,SLOT(slot_updateUiByRes(QFileInfoList)));
}

void MainWindow::slot_setUpdateFlag()
{
    /*
     * This operation setted because that inotify event send no more one siganl.
     * So set a 500ms duration to ignore theres no-use siganls.
     * Note: it is expected to optimize.
     */
    if(!mediaHasUpdate)
    {
        mediaHasUpdate = true;
        QTimer::singleShot(500,this,SLOT(slot_updateMedia()));
    }
}

void MainWindow::slot_updateMedia()
{
    qDebug()<<"Update media resource.";
    mediaUpdateThread *thread = new mediaUpdateThread(this,this);
    thread->start();
    mediaHasUpdate = false;
}

void MainWindow::slot_updateUiByRes(QFileInfoList videoFileList)
{
    m_videoWid->updateUiByRes(videoFileList);
}

void MainWindow::disableApplication()
{
    qDebug("disable video application.");
    m_videoWid->setPlayerPause();
    this->setVisible(false);
}

void MainWindow::enableApplication()
{
    qDebug("enable video application.");
    this->setVisible(true);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    qDebug()<<"Received ketpress event with key value:"<<event->key();
    switch(event->key())
    {
    case Qt::Key_VolumeDown:
        m_videoWid->updateVolume(false);;
        break;
    case Qt::Key_VolumeUp:
        m_videoWid->updateVolume(true);
        break;
    case Qt::Key_PowerOff:
        // when key_power enter
        m_videoWid->setPlayerPause();
        QTimer::singleShot(100, this, SLOT(slot_standby()));
        break;
    default:
        break;
    }
    QWidget::keyPressEvent(event);
}

void MainWindow::slot_standby()
{
}

mediaUpdateThread::mediaUpdateThread(QObject *parent,MainWindow *mainWindow):QThread(parent)
{
    m_mainWindow = mainWindow;
    qRegisterMetaType<QFileInfoList>("QFileInfoList");
}
void mediaUpdateThread::run()
{
    QFileInfoList videoFileList = m_mainWindow->getVideoWidget()->findAllVideoFiles(VIDEO_SEARCH_PATH);
    emit m_mainWindow->updateUiByRes(videoFileList);
}

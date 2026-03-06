#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "appconfigbase.h"
#include "appeventbase.h"
#include <QWidget>
#include <QScroller>
#include <QProcess>
#include "appdatabasebase.h"
#include "appcommonbase.h"
#include <QDesktopWidget>
#include "dbthread.h"
#include "logmanager.h"
#include <QGraphicsDropShadowEffect>
#include <QDir>
#include <QFileInfo>
#include <QDirIterator>

static QList<QFileInfo> getLogFiles(const QString& dirPath) {
    QList<QFileInfo> logFiles;
    QDirIterator it(dirPath, QStringList() << "*.xyp"<<"ProjectInfo*", QDir::Files, QDirIterator::Subdirectories);

    while (it.hasNext())  {
        logFiles.append(QFileInfo(it.next()));
    }
    return logFiles;
}

static QList<QFileInfo> getJPGFiles(const QString& dirPath) {
    QList<QFileInfo> logFiles;
    QDirIterator it(dirPath, QStringList() << "*.jpg", QDir::Files, QDirIterator::Subdirectories);

    while (it.hasNext())  {
        logFiles.append(QFileInfo(it.next()));
    }
    return logFiles;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qDebug() << "1";

    //初始化
    init();

    qDebug() << "2";

    //创建信号槽连接
    createConnect();

    qDebug() << "3";
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(nullptr != dbthread)
    {
        dbthread->stopThread();    // 也可以使用thread->exit(0);
        dbthread->quit();
        dbthread->wait();
    }
    LogManager::getInstance()->stopThread();
    emit sig_mainWindowQuit();
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    //设置相机的缓存数量
    QTimer::singleShot(1500, [=]() {
        ui->camera0->getHikCamera().setCacheCount(AppConfigBase::getInstance()->readCameraSettings("linearArrayCameraParams","cacheThreadCount","1").toInt());

        if(nullptr == dbthread)
        {
            dbthread = new DBThread;
            dbthread->start();
            connect(AppEventBase::getInstance(), &AppEventBase::sig_requestServer, dbthread, &DBThread::saveDB,Qt::DirectConnection);
        }

        LogManager::getInstance()->startThread();//启动log日志
    });

    QTimer::singleShot(2000, [=]() {
        ui->camera3->getHikCamera().setCacheCount(AppConfigBase::getInstance()->readCameraSettings("linearArrayCameraParams","cacheThreadCount","1").toInt());
    });

    QTimer::singleShot(3000, [=]() {
        ui->camera1->getHikCamera().setCacheCount(AppConfigBase::getInstance()->readCameraSettings("areaArrayCameraParams","cacheThreadCount","1").toInt());
    });

    updateLayout();
    QTimer::singleShot(200, [=]() {
        updateLayout();
    });
}

void MainWindow::updateLayout()
{
    const int cameraWidth = AppConfigBase::getInstance()->readCameraSettings("CameraName","linearArrayCamera_R_width","2048").toInt();
    const int overlapWidth = AppConfigBase::getInstance()->readCameraSettings("CameraName","linearArrayCamera_overlap","1048").toInt(); // 重叠区域
    const int cameraHeight = AppConfigBase::getInstance()->readCameraSettings("CameraName","linearArrayCamera_R_height","2000").toInt(); // 两个相机高度相同

    //图片总宽度为：
    double cameraAllWidth = (cameraWidth-overlapWidth)*2 + overlapWidth;

    //计算左右两个区域应该显示的实际宽度
    double endCameraRadius = double(cameraHeight)/cameraAllWidth;
    double endPixAllWidth = ui->widget->height()/endCameraRadius;
    double eveDisplayWidth = endPixAllWidth/2;
    double radius = double(cameraHeight)/cameraWidth;

    //等比例缩放后，右侧相机实际显示到界面的宽度计算
    double realPix_width_R = ui->widget->height()/radius;

    //右侧相机先显示处理
    ui->widgetCamera3->setGeometry(ui->widget->width()-realPix_width_R,0,realPix_width_R,ui->widget->height());
    ui->camera3->setGeometry(0,0,ui->widgetCamera3->width(),ui->widgetCamera3->height());

    //总相机宽度÷2,计算右侧相机应该向左平移多少像素
    double traslatePixWidth_R = double(ui->widget->width())/2 - eveDisplayWidth;
    ui->widgetCamera3->setGeometry(ui->widget->width()-realPix_width_R-traslatePixWidth_R,0,ui->widgetCamera3->width(),ui->widgetCamera3->height());

    //左侧相机默认显示区域的一半
    ui->widgetCamera0->setGeometry(0,0,ui->widget->width()/2,ui->widget->height());
    ui->camera0->setGeometry(0,0,realPix_width_R,ui->widget->height());

    //计算左侧相机应该向右平移多少像素
    ui->camera0->setGeometry(0+traslatePixWidth_R,0,realPix_width_R,ui->widget->height());

    ui->widgetCamera0->raise();

    //左相机上下的平移数(上正下负)
    if(!AppConfigBase::getInstance()->readCameraSettings("CameraName","linearArrayCamera_L_offsetY","").isEmpty())
    {
        double right_offsetY = AppConfigBase::getInstance()->readCameraSettings("CameraName","linearArrayCamera_L_offsetY","15").toDouble();; //平移像素个数
        double radius = static_cast<double>(right_offsetY) / cameraHeight;
        double offsetY_pix = ui->widgetCamera0->height()*radius;
        ui->widgetCamera0->setGeometry(ui->widgetCamera0->geometry().x(),ui->widgetCamera0->geometry().y()-offsetY_pix,
                            ui->widgetCamera0->geometry().width(),ui->widgetCamera0->geometry().height());

        //向上平移，下边增加一个遮罩
        static QLabel* upTask = new QLabel(ui->widget);
        upTask->setStyleSheet("background-color:#F8F8F4");
//        upTask->setStyleSheet("background-color:rgb(255,0,0)");
        if(offsetY_pix > 0)
        {
            upTask->setGeometry(0,ui->widget->height()-offsetY_pix,ui->widget->width(),offsetY_pix+2);
            upTask->raise();
            upTask->show();
        }
        else if(offsetY_pix < 0)
        {
            upTask->setGeometry(0,0,ui->widget->width(),fabs(offsetY_pix)+2);
            upTask->raise();
            upTask->show();
        }
        else
        {
            upTask->hide();
        }
    }

    //右相机上下的平移数(上正下负) 建议右相机向上平移7.5，左相机向下平移7.5
    if(!AppConfigBase::getInstance()->readCameraSettings("CameraName","linearArrayCamera_R_offsetY","").isEmpty())
    {
        double right_offsetY = AppConfigBase::getInstance()->readCameraSettings("CameraName","linearArrayCamera_R_offsetY","15").toDouble();; //平移像素个数
        double radius = static_cast<double>(right_offsetY) / cameraHeight;
        double offsetY_pix = ui->widgetCamera3->height()*radius;
        ui->widgetCamera3->setGeometry(ui->widgetCamera3->geometry().x(),ui->widgetCamera3->geometry().y()-offsetY_pix,
                            ui->widgetCamera3->geometry().width(),ui->widgetCamera3->geometry().height());

        //向上平移，下边增加一个遮罩
        static QLabel* downTask = new QLabel(ui->widget);
        downTask->setStyleSheet("background-color:#F8F8F4");
//        downTask->setStyleSheet("background-color:rgb(255,0,0)");
        if(offsetY_pix > 0)
        {
            downTask->setGeometry(0,ui->widget->height()-offsetY_pix,ui->widget->width(),offsetY_pix+2);
            downTask->raise();
            downTask->show();
        }
        else if(offsetY_pix < 0)
        {
            downTask->setGeometry(0,0,ui->widget->width(),fabs(offsetY_pix)+2);
            downTask->raise();
            downTask->show();
        }
        else
        {
            downTask->hide();
        }
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    return QMainWindow::eventFilter(watched, event);
}

/**
 * @brief slt_prepareBtnClicked 准备按钮点击事件
 */
void MainWindow::slt_prepareBtnClicked()
{
    //点击准备后，不可修改参数信息
    m_settingDialog.slt_prepareBtnClicked();
    QApplication::processEvents();

    //1. 开启道路相机并创建回调
    if(ui->statusBar->getCamera0()->isVisible())
    {
        ui->camera0->openCamera(CAMERA_TYPE::CL042);
    }

    //1. 开启道路相机并创建回调
    if(ui->statusBar->getCamera3()->isVisible())
    {
        ui->camera3->openCamera(CAMERA_TYPE::CL042_2);
    }

    //2. 开启景观相机并创建回调
    if(ui->statusBar->getCamera1()->isVisible())
    {
        ui->camera1->openCamera(CAMERA_TYPE::CH120);
    }

    //3.开启和平整度测试仪器直接的连接，并创建回调
    ui->acceleration->hide();
    ui->planeness->hide();
    if(ui->statusBar->getFlatnessTester()->isVisible())
    {
        m_flatnessTester.startConnect();
        ui->planeness->start();
        ui->planeness->show();
    }

    //3.开启和加速度计的连接，并创建回调
    if(ui->statusBar->getAccelerometer()->isVisible())
    {
        m_accelerometer.startConnect();
        ui->acceleration->start();
        ui->acceleration->show();
    }

    //4.1 开启地图显示、开启GPS仪器的信号接收
    if(ui->statusBar->getGps()->isVisible())
    {
        ui->mapPage->loadMapUrl(qApp->applicationDirPath() + "/tianditu.html",m_settingDialog.getGpsPort());
    }

    //开始准备后，菜单按钮激活
    ui->startSaveBtn->setEnabled(true);
    ui->checkPileNumBtn->setEnabled(true);
    ui->eventMarkBtn->setEnabled(true);
    //事件按钮鼠标穿透
    ui->widget_opt->setAttribute(Qt::WA_TransparentForMouseEvents,false);
    ui->settingBtn->setEnabled(false);
    ui->prepareBtn->setEnabled(false);
}

/**
 * @brief slt_startSaveBtnClicked 开始存储按钮点击事件
 */
void MainWindow::slt_startSaveBtnClicked()
{
    //是否已经点击开始存储
    if(m_isStartSave)
        return;

    m_isStartSave = true;
    qDebug() << "点击开始存储 start";
    ui->startSaveBtn->setChecked(true);
    ui->startSaveBtn->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->stopSaveBtn->setEnabled(true);
    ui->mapPage->startRecord(true);
    //调用道路相机和景观相机的sdk的存储功能，并设置存储路径
    if(ui->statusBar->getCamera0()->isVisible())
    {
        ui->camera0->startRecord(m_settingDialog.getSaveFolderPath());
        qDebug() << "线阵相机准备就绪";
    }
    if(ui->statusBar->getCamera3()->isVisible())
    {
        ui->camera3->startRecord(m_settingDialog.getSaveFolderPath());
        qDebug() << "线阵相机准备就绪";
    }
    if(ui->statusBar->getCamera1()->isVisible())
    {
        ui->camera1->startRecord(m_settingDialog.getSaveFolderPath());
        qDebug() << "面阵相机准备就绪";
    }
    m_accelerometer.startRecord(true);
    m_flatnessTester.startRecord(true);
    ui->statusBar->setStartRecord();
    ui->mapPage->resetGPS();//重置脉冲
    QApplication::processEvents();
    qDebug() << "点击开始存储 end";

    ui->prepareBtn->setEnabled(false);
    ui->startSaveBtn->setEnabled(false);
}

void MainWindow::slt_stopSaveBtnClicked()
{
    if(!ui->startSaveBtn->isChecked())
    {
        return;
    }

    //先停止存储，在关闭设备
    qDebug() << "点击结束存储 start";
    AppCommonBase::getInstance()->bIsGPSReset = false;
    qDebug() << "停止存储标志位设置成功";
    ui->camera0->getHikCamera().closeSaveImage();
    ui->camera3->getHikCamera().closeSaveImage();
    ui->camera1->getHikCamera().closeSaveImage();
    m_accelerometer.startRecord(false);
    m_flatnessTester.startRecord(false);
    ui->camera0->stopRecord();
    ui->camera3->stopRecord();
    ui->camera1->stopRecord();
    //ui->mapPage->startRecord(false);

    //等待2秒，为了多存gps数据
    QEventLoop loop;
    QTimer timer;
    timer.setInterval(2000);  // 设置超时时间 3 秒
    timer.setSingleShot(true);  // 单次触发
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start();
    loop.exec();

    qDebug() << "线阵相机关闭 start";
    slt_openCamera0(false);
    qDebug() << "线阵相机关闭 end";
    qDebug() << "面阵相机关闭 start";
    slt_openCamera1(false);
    qDebug() << "面阵相机关闭 end";
    slt_openFlatnessTester(false);
    slt_openAccelerometer(false);
    slt_openGPS(false);
    ui->mapPage->startRecord(false);
    ui->startSaveBtn->setChecked(false);
    ui->startSaveBtn->setEnabled(false);
    ui->stopSaveBtn->setEnabled(false);
    ui->prepareBtn->setEnabled(false);
    qDebug() << "点击结束存储 end";

    QApplication::processEvents();
    //解析文件，修改公司名称
    changeFilesCompany();

    //输出统计报告
    outputRecord();
}

void MainWindow::slt_paramsSaveSuccessed(QJsonObject paramsObj)
{
    m_flatnessTester.slt_setParamsInfo(paramsObj);
    ui->prepareBtn->setEnabled(true);

    //设备图标隐藏
    ui->statusBar->getCamera0()->setVisible(false);
    ui->statusBar->getCamera3()->setVisible(false);
    ui->statusBar->getCamera1()->setVisible(false);
    ui->statusBar->getFlatnessTester()->setVisible(false);
    ui->statusBar->getAccelerometer()->setVisible(false);
    ui->statusBar->getGps()->setVisible(false);
    ui->camera0->setVisible(false);
    ui->camera3->setVisible(false);

    if(paramsObj.value("camera0Flag").toBool())
    {
         ui->statusBar->getCamera0()->setVisible(true);
         ui->camera0->setVisible(true);
    }
    if(paramsObj.value("camera1Flag").toBool())
    {
         ui->statusBar->getCamera1()->setVisible(true);
    }
    if(paramsObj.value("camera3Flag").toBool())
    {
         ui->statusBar->getCamera3()->setVisible(true);
         ui->camera3->setVisible(true);
    }
    if(paramsObj.value("flatnessTesterFlag").toBool())
    {
         ui->statusBar->getFlatnessTester()->setVisible(true);
    }
    if(paramsObj.value("accelerometerFlag").toBool())
    {
         ui->statusBar->getAccelerometer()->setVisible(true);
    }
    if(paramsObj.value("gpsFlag").toBool())
    {
         ui->statusBar->getGps()->setVisible(true);
    }

    if(ui->camera0->isVisible() && ui->camera0->isVisible())
    {
        AppCommonBase::getInstance()->m_isDoubleLinearArrayCamera = true;
    }
    else
    {
        AppCommonBase::getInstance()->m_isDoubleLinearArrayCamera = false;
    }

    ui->statusBar->getBtnWidget()->setVisible(true);
}

/**
 * @brief createConnect 创建信号槽连接
 */
void MainWindow::createConnect()
{
    //[配置工程/选择设备]按钮点击后，弹出设置窗口
    connect(ui->settingBtn, &QPushButton::clicked, &m_settingDialog, &FormSetting::show);
    //[准备]按钮点击后
    connect(ui->prepareBtn, &QPushButton::clicked, this, &MainWindow::slt_prepareBtnClicked);
    //[开始存储]按钮点击后
    connect(ui->startSaveBtn, &QPushButton::clicked, this, &MainWindow::slt_startSaveBtnClicked);
    //[停止存储]按钮点击后
    connect(ui->stopSaveBtn, &QPushButton::clicked, this, &MainWindow::slt_stopSaveBtnClicked);
    //[校桩]按钮点击后
    ui->checkPileNumBtn->setShortcut(QKeySequence(QLatin1String("F8")));
    connect(ui->checkPileNumBtn, &QPushButton::clicked, &m_formPileNumUpdate, &FormPileNumUpdate::show);
    //[事件标注]按钮点击后
    ui->eventMarkBtn->setShortcut(QKeySequence(QLatin1String("F9")));
    connect(ui->eventMarkBtn, &QPushButton::clicked, &m_formEventMark, &FormEventMark::show);
    //[退出软件]按钮点击后
    connect(ui->exitBtn, &QPushButton::clicked, this, &MainWindow::close);
    //接收GPS的信号/平整度仪的信号，显示到状态栏中
    connect(ui->mapPage, &WidgetMapPage::sig_sendGPSData, ui->statusBar, &WidgetStatusBar::slt_setGPSData);
    //connect(&m_flatnessTester, &FlatnessTester::sig_sendGNSSInfo, ui->statusBar, &WidgetStatusBar::slt_setGPSData);
    //设置弹窗中的信息，显示到状态栏中
    connect(&m_settingDialog, &FormSetting::sig_sendParams, ui->statusBar, &WidgetStatusBar::slt_setGPSData);
    connect(&m_settingDialog, &FormSetting::sig_sendParams, this, &MainWindow::slt_paramsSaveSuccessed);
    //追加log日志
//    connect(AppEventBase::getInstance(), &AppEventBase::sig_sendServerMsg, ui->logEdit, &QTextEdit::append,Qt::AutoConnection);
    connect(AppEventBase::getInstance(),&AppEventBase::sig_sendServerMsg,this,[=](QString msg){
        if(!msg.contains(QDateTime::currentDateTime().toString("yyyy-MM-dd")))
            msg = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + msg;

        if(msg.length() > 11)
            msg = msg.mid(10);

        ui->logEdit->moveCursor(QTextCursor::End);
        ui->logEdit->ensureCursorVisible();                  // 确保光标可见（自动滚动）
        QString insertText = QString("<span style=\" font-size:20px; color:#00ff00;\">• </span><span style=\" font-size:16px;\">%1<br></span>").arg(msg);
        ui->logEdit->insertHtml(insertText);
        ui->logEdit->update();
    },Qt::AutoConnection);
    connect(AppEventBase::getInstance(),&AppEventBase::sig_sendServerMsg,this,[=](QString msg){
        qDebug() << msg;
        return;
        QJsonObject obj;
        obj.insert("text",msg);
        AppEventBase::getInstance()->sig_requestServer(SAVE_LOG,obj);
    },Qt::AutoConnection);
//    //右侧操作按钮点击事件
    connect(ui->statusBar->getCamera0(), &QToolButton::clicked, this, &MainWindow::slt_Camera0Clicked);
    connect(ui->statusBar->getCamera1(), &QToolButton::clicked, this, &MainWindow::slt_Camera1Clicked);
    connect(ui->statusBar->getCamera3(), &QToolButton::clicked, this, &MainWindow::slt_Camera3Clicked);
    connect(ui->camera0, &MemoryImage::sig_mouseDoubleClicked, this, &MainWindow::slt_cameraDoubleClicked);
    connect(ui->camera3, &MemoryImage::sig_mouseDoubleClicked, this, &MainWindow::slt_cameraDoubleClicked);
    connect(ui->camera1, &MemoryImage::sig_mouseDoubleClicked, this, &MainWindow::slt_cameraDoubleClicked);

    connect(ui->camera3, &MemoryImage::sig_exposureTimeValueChanged, &ui->camera0->getHikCamera(), &HikCamera::setExposureTime);
    connect(ui->camera3, &MemoryImage::sig_gainValueChanged, &ui->camera0->getHikCamera(), &HikCamera::setgainValue);
    ui->camera0->resetCameraParent(CAMERA_TYPE::CL042);
    ui->camera3->resetCameraParent(CAMERA_TYPE::CL042_2);
    ui->camera1->resetCameraParent(CAMERA_TYPE::CH120);
//    connect(ui->statusBar->getFlatnessTester(), &QToolButton::clicked, this, &MainWindow::slt_openFlatnessTester);
//    connect(ui->statusBar->getGps(), &QToolButton::clicked, this, &MainWindow::slt_openGPS);
    //更新里程
    connect(AppEventBase::getInstance(), &AppEventBase::sig_updateMileage, this, &MainWindow::slt_updateMileage);

    AppDatabaseBase::getInstance()->g_chart = ui->acceleration;
    connect(&m_flatnessTester, &FlatnessTester::sig_sendValue, ui->planeness, &MyFlatnessTesterChart::slt_addValues,Qt::DirectConnection);
//    connect(&m_accelerometer, &Accelerometer::sig_sendValue, ui->acceleration, &ChartWidget::slt_drawLine);

    //路面事件触发
    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::slt_roadEventClicked);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &MainWindow::slt_roadEventClicked);
    connect(ui->pushButton_3, &QPushButton::clicked, this, &MainWindow::slt_roadEventClicked);
    //非路面事件触发
    connect(ui->pushButton_4, &QToolButton::clicked, this, &MainWindow::slt_notroadEventClicked);
    connect(ui->pushButton_5, &QToolButton::clicked, this, &MainWindow::slt_notroadEventClicked);
    connect(ui->pushButton_6, &QToolButton::clicked, this, &MainWindow::slt_notroadEventClicked);
    connect(ui->pushButton_7, &QToolButton::clicked, this, &MainWindow::slt_notroadEventClicked);
    connect(ui->pushButton_8, &QToolButton::clicked, this, &MainWindow::slt_notroadEventClicked);

    //桩号更新
    connect(AppEventBase::getInstance(), &AppEventBase::sig_requestServer, this, &MainWindow::slt_pileNumUpdated);
}

/**
 * @brief init 类的初始化
 */
void MainWindow::init()
{
    this->setWindowFlags(Qt::FramelessWindowHint);
    //1. 软件开机后，只有配置和退出软件功能能使用，其它暂时禁用
    ui->prepareBtn->setEnabled(false);
    ui->startSaveBtn->setEnabled(false);
    ui->stopSaveBtn->setEnabled(false);
    ui->checkPileNumBtn->setEnabled(false);
    ui->eventMarkBtn->setEnabled(false);
    ui->statusBar->getFlatnessTester()->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->statusBar->getAccelerometer()->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->statusBar->getGps()->setAttribute(Qt::WA_TransparentForMouseEvents);

    //设备图标隐藏
    ui->statusBar->getCamera0()->setVisible(false);
    ui->statusBar->getCamera3()->setVisible(false);
    ui->statusBar->getCamera1()->setVisible(false);
    ui->statusBar->getFlatnessTester()->setVisible(false);
    ui->statusBar->getAccelerometer()->setVisible(false);
    ui->statusBar->getGps()->setVisible(false);

    //触摸滑动
    QScroller::grabGesture(ui->logEdit,QScroller::LeftMouseButtonGesture);
    //QScroller::grabGesture(ui->pileNum_edit,QScroller::LeftMouseButtonGesture);
    ui->tableWidget->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setVisible(true);
    //获取主屏幕分辨率
    QRect screenRect = QApplication::desktop()->screenGeometry();
    if(screenRect.height() > 1400)
    {
        ui->tableWidget->setRowCount(10);
    }
    else
    {
        ui->tableWidget->setRowCount(8);
    }

//    ui->tableWidget->horizontalHeader()->setStyleSheet("    QHeaderView::section\
//    {\
//        min-height:50px;\
//        max-height:50px;\
//        margin-left:0px;\
//        padding-left:4px;\
//        border:none;\
//        background-color: rgba(151, 208, 250,110);\
//    }");

    //事件按钮鼠标穿透
    ui->widget_opt->setAttribute(Qt::WA_TransparentForMouseEvents,true);
}

void MainWindow::changeFilesCompany()
{
    QString folderPath = m_settingDialog.getSaveFolderPath();
    QList<QFileInfo> files = getLogFiles(folderPath);
    for (const QFileInfo& tempFile : files) {
        //qDebug() << "Path:" << tempFile.absoluteFilePath()  << "Name:" << tempFile.fileName();

        if(tempFile.fileName() == "IRI.xyp" || tempFile.fileName() == "PB.xyp" || tempFile.fileName().contains("ProjectInfo"))
        {
            QFile file(tempFile.absoluteFilePath());
            if (file.open(QIODevice::ReadWrite  | QIODevice::Text)) {{
                QTextStream stream(&file);
                QStringList lines;
                while (!stream.atEnd())  {{
                    QString line = stream.readLine().replace("熙赢测控",  "山东高速");
                    lines.append(line);
                }}
                file.resize(0);  // 清空原文件
                for (const QString& line : lines) {{
                    stream << line << "\n";
                }}
                file.close();
            }}
        }
    }
}

void MainWindow::outputRecord()
{
   // ①gpposdmi1,type=1个数；②面阵相机图像个数 & 上升沿/真开始/zhen结束消息；③ 破损左相机(-025)图像个数；③ 破损右相机(-025)图像个数；④log中产生异常的消息。
    qDebug() << "-----------------------------------------------------------";
    AppEventBase::getInstance()->sig_requestServer(SLECT_GPPOSTMI,QJsonObject());

    QString folderPath = m_settingDialog.getSaveFolderPath();
    //获取面阵相机个数
    if(!ui->camera1->getHikCamera().getCameraName().isEmpty())
    {
        QString savePath = folderPath + "/" + ui->camera1->getHikCamera().getCameraName();
        QList<QFileInfo> files = getJPGFiles(savePath);
        qDebug() << "景观相机存图个数:" << QString::number(files.size());
        if(!ui->camera1->getHikCamera().getLastMsg().isEmpty())
        {
            qDebug() << ui->camera1->getHikCamera().getLastMsg();
        }
    }
    //获取破损相机(左)
    if(!ui->camera0->getHikCamera().getCameraName().isEmpty())
    {
        QString savePath = folderPath + "/" + ui->camera0->getHikCamera().getCameraName();
        QList<QFileInfo> files = getJPGFiles(savePath);
        qDebug() << "破损相机(左)存图个数:" << QString::number(files.size());
    }
    //获取破损相机(右)
    if(!ui->camera3->getHikCamera().getCameraName().isEmpty())
    {
        QString savePath = folderPath + "/" + ui->camera3->getHikCamera().getCameraName();
        QList<QFileInfo> files = getJPGFiles(savePath);
        qDebug() << "破损相机(右)存图个数:" << QString::number(files.size());
    }
    qDebug() << "程序运行过程中的异常信息,请全局搜索[异常]该关键词";
    qDebug() << "-----------------------------------------------------------";
}

void MainWindow::slt_openCamera0(bool isChecked)
{
    if(isChecked)
    {
        ui->camera0->openCamera(CAMERA_TYPE::CL042);
        ui->camera3->openCamera(CAMERA_TYPE::CL042_2);
    }
    else
    {
        ui->camera0->closeCamera();
        ui->camera3->closeCamera();
    }
}

void MainWindow::slt_Camera0Clicked()
{
    ui->camera3->openCameraParmas();
}

void MainWindow::slt_Camera3Clicked()
{
    ui->camera3->openCameraParmas();
}


void MainWindow::slt_cameraDoubleClicked()
{
    if(ui->camera0 == sender())
    {
        if("max" == ui->camera0->property("status").toString())
        {
            ui->widgetmianCamera->setVisible(true);
            ui->seriesWidget->setVisible(true);
            ui->widgetMap->setVisible(true);
            ui->gridLayout->setRowStretch(0, 1);
            ui->gridLayout->setRowStretch(1, 1);
            ui->gridLayout->setColumnStretch(0, 1);
            ui->gridLayout->setColumnStretch(1, 1);

            ui->camera0->setProperty("status","normal");
            ui->camera3->setProperty("status","normal");
        }
        else
        {
            ui->gridLayout->setRowStretch(0, 0);
            ui->gridLayout->setRowStretch(1, 0);
            ui->gridLayout->setColumnStretch(0, 0);
            ui->gridLayout->setColumnStretch(1, 0);
            ui->widgetmianCamera->setVisible(false);
            ui->seriesWidget->setVisible(false);
            ui->widgetMap->setVisible(false);
            ui->camera0->setProperty("status","max");
            ui->camera3->setProperty("status","max");
        }
    }

    if(ui->camera1 == sender())
    {
        if("max" == ui->camera1->property("status").toString())
        {
            ui->widgetLineCamera->setVisible(true);
            ui->widgetmianCamera->setVisible(true);
            ui->seriesWidget->setVisible(true);
            ui->widgetMap->setVisible(true);
            ui->gridLayout->setRowStretch(0, 1);
            ui->gridLayout->setRowStretch(1, 1);
            ui->gridLayout->setColumnStretch(0, 1);
            ui->gridLayout->setColumnStretch(1, 1);

            ui->camera1->setProperty("status","normal");
        }
        else
        {
            ui->widgetmianCamera->setVisible(true);
            ui->gridLayout->setRowStretch(0, 0);
            ui->gridLayout->setRowStretch(1, 0);
            ui->gridLayout->setColumnStretch(0, 0);
            ui->gridLayout->setColumnStretch(1, 0);
            ui->widgetLineCamera->setVisible(false);
            ui->seriesWidget->setVisible(false);
            ui->widgetMap->setVisible(false);
            ui->camera1->setProperty("status","max");
        }
    }

    if(ui->camera3 == sender())
    {
        if("max" == ui->camera3->property("status").toString())
        {
            ui->widgetmianCamera->setVisible(true);
            ui->seriesWidget->setVisible(true);
            ui->widgetMap->setVisible(true);
            ui->gridLayout->setRowStretch(0, 1);
            ui->gridLayout->setRowStretch(1, 1);
            ui->gridLayout->setColumnStretch(0, 1);
            ui->gridLayout->setColumnStretch(1, 1);

            ui->camera0->setProperty("status","normal");
            ui->camera3->setProperty("status","normal");
        }
        else
        {
            ui->gridLayout->setRowStretch(0, 0);
            ui->gridLayout->setRowStretch(1, 0);
            ui->gridLayout->setColumnStretch(0, 0);
            ui->gridLayout->setColumnStretch(1, 0);
            ui->widgetmianCamera->setVisible(false);
            ui->seriesWidget->setVisible(false);
            ui->widgetMap->setVisible(false);

            ui->camera0->setProperty("status","max");
            ui->camera3->setProperty("status","max");
        }
    }

    QApplication::processEvents();

//    int margin = 0;
//    int w = ui->widget->width();
//    int h = ui->widget->height();
//    int cameraWidth = AppConfigBase::getInstance()->readCameraSettings("CameraName","linearArrayCamera_R_width","2048").toInt();
//    int OffsetX = AppConfigBase::getInstance()->readCameraSettings("CameraName","linearArrayCamera_R_OffsetX","").toInt();
//    double radius = double(OffsetX)/cameraWidth;
//    double xxx = double(w-margin*2)/(1+1-radius);
//    ui->widgetCamera0->setGeometry(margin,margin,xxx,(h-margin*2));
//    ui->widgetCamera3->setGeometry(9+xxx*(1-radius),margin,xxx,(h-margin*2));
//    QApplication::processEvents();

    updateLayout();
}

void MainWindow::slt_openCamera1(bool isChecked)
{
    if(isChecked)
    {
        ui->camera1->openCamera(CAMERA_TYPE::CH120);
    }
    else
    {
        ui->camera1->closeCamera();
    }
}

void MainWindow::slt_Camera1Clicked()
{
    ui->camera1->openCameraParmas();
}

void MainWindow::slt_openFlatnessTester(bool isChecked)
{
    if(isChecked)
    {
        m_flatnessTester.startConnect();
    }
    else
    {
        m_flatnessTester.stopConnect();
    }
}

void MainWindow::slt_openAccelerometer(bool isChecked)
{
    if(isChecked)
    {
        m_accelerometer.startConnect();
    }
    else
    {
        m_accelerometer.stopConnect();
    }
}

void MainWindow::slt_openGPS(bool isChecked)
{
    if(isChecked)
    {
        ui->mapPage->loadMapUrl(qApp->applicationDirPath() + "/tianditu.html",m_settingDialog.getGpsPort());
    }
    else
    {
        ui->mapPage->closeGPSRcv();
    }
}

void MainWindow::slt_updateMileage(double value)
{
    int startMileage = m_settingDialog.getStartMileage();
    int currentMileage = startMileage + int(value);

    //计算当前的桩号
    int kmValue = int(currentMileage)/1000;
    int mValue = int(currentMileage)%1000;
}

void MainWindow::slt_roadEventClicked()
{
    QApplication::processEvents();
    QPushButton* f_sender = dynamic_cast<QPushButton*>(sender());
    if(nullptr != f_sender)
    {
        QString marker = f_sender->text();
        QString openTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        QJsonObject gpsObj = AppCommonBase::getInstance()->getLastGPSPoint();

        if("沥青路" == marker)
            AppCommonBase::getInstance()->g_roadType = "a";
        else if("水泥路" == marker)
            AppCommonBase::getInstance()->g_roadType = "c";
        else if("砂石路" == marker)
            AppCommonBase::getInstance()->g_roadType = "g";

        //参数信息保存到数据库
        QJsonObject objParams;
        objParams.insert("marker",marker);//事件标记
        objParams.insert("type","0");//0：路面类型事件
        objParams.insert("lon",gpsObj.value("lon").toDouble());//经度
        objParams.insert("lat",gpsObj.value("lat").toDouble());//纬度
        objParams.insert("locationId",gpsObj.value("id").toString());//经纬度数据的唯一标识
        objParams.insert("createTime",openTime);//打开对话框的时间
        int pulse_value = gpsObj.value("pulse_value").toInt();
        objParams.insert("pulse_value",pulse_value);//脉冲值
        objParams.insert("current_distance",gpsObj.value("distance").toDouble());//当前里程
        objParams.insert("GPS_time",gpsObj.value("GPS_time").toString());//当前gps时间
        objParams.insert("current_station",AppCommonBase::getInstance()->g_currentStation);//当前桩号
        AppEventBase::getInstance()->sig_requestServer(SAVE_EVENT_MARK,objParams);
    }
}

void MainWindow::slt_notroadEventClicked()
{
    QApplication::processEvents();
    QToolButton* f_sender = dynamic_cast<QToolButton*>(sender());
    if(nullptr != f_sender)
    {
        QString openTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        QJsonObject gpsObj = AppCommonBase::getInstance()->getLastGPSPoint();

        QString marker = f_sender->text();
        if(f_sender->isChecked())
            marker += "开始";
        else
        {
            marker += "结束";
        }

        //参数信息保存到数据库
        QJsonObject objParams;
        objParams.insert("marker",marker);//事件标记
        objParams.insert("type","1");//1：非路面类型事件
        objParams.insert("lon",gpsObj.value("lon").toDouble());//经度
        objParams.insert("lat",gpsObj.value("lat").toDouble());//纬度
        objParams.insert("locationId",gpsObj.value("id").toString());//经纬度数据的唯一标识
        objParams.insert("createTime",openTime);//打开对话框的时间
        int pulse_value = gpsObj.value("pulse_value").toInt();
        objParams.insert("pulse_value",pulse_value);//脉冲值
        objParams.insert("current_distance",gpsObj.value("distance").toDouble());//当前里程
        objParams.insert("GPS_time",gpsObj.value("GPS_time").toString());//当前gps时间
        objParams.insert("current_station",AppCommonBase::getInstance()->g_currentStation);//当前桩号
        AppEventBase::getInstance()->sig_requestServer(SAVE_EVENT_MARK,objParams);
    }
}

void MainWindow::slt_pileNumUpdated(QString url, QJsonObject obj)
{
    if(UPDATE_PILENUM != url)
        return;

    QString createTime = QDateTime::fromString(obj.value("createTime").toString(),"yyyy-MM-dd hh:mm:ss.zzz").toString("hh:mm:ss");
    QString pulse_value = QString::number(obj.value("pulse_value").toInt());
    QString current_distance = QString::number(obj.value("current_distance").toDouble());
    QString pileNum = obj.value("pileNum").toString();

    //获取当前的里程
    ui->statusBar->setStartMileageAndDistance(pileNum.toDouble()*1000,current_distance.toDouble());

    int rowCount = ui->tableWidget->rowCount();
    //单元格被填充满了
    if(nullptr != ui->tableWidget->item(rowCount-1,0) && !ui->tableWidget->item(rowCount-1,0)->text().isEmpty())
    {
        for(int index = 1; index < rowCount; index++)
        {
            ui->tableWidget->item(index-1,0)->setText(ui->tableWidget->item(index,0)->text());
            ui->tableWidget->item(index-1,1)->setText(ui->tableWidget->item(index,1)->text());
            ui->tableWidget->item(index-1,2)->setText(ui->tableWidget->item(index,2)->text());
            ui->tableWidget->item(index-1,3)->setText(ui->tableWidget->item(index,3)->text());
        }
        ui->tableWidget->item(rowCount-1,0)->setText(createTime);
        ui->tableWidget->item(rowCount-1,1)->setText(pulse_value);
        ui->tableWidget->item(rowCount-1,2)->setText(QString::number(current_distance.toDouble(),'f',3));
        ui->tableWidget->item(rowCount-1,3)->setText(QString::number(pileNum.toDouble(),'f',3));
    }
    else
    {
        for(int index = 0; index < rowCount; index++)
        {
            //单元格未创建
            if(nullptr == ui->tableWidget->item(index,0))
            {
                ui->tableWidget->setItem(index,0,new QTableWidgetItem(createTime));
            }
            else
            {
                //单元格中没有内容
                if(ui->tableWidget->item(index,0)->text().isEmpty())
                {
                    ui->tableWidget->item(index,0)->setText(createTime);
                }
                else
                {
                    continue;
                }
            }

            if(nullptr == ui->tableWidget->item(index,1))
            {
                ui->tableWidget->setItem(index,1,new QTableWidgetItem(pulse_value));
            }
            else
            {
                //单元格中没有内容
                if(ui->tableWidget->item(index,1)->text().isEmpty())
                {
                    ui->tableWidget->item(index,1)->setText(pulse_value);
                }
            }

            if(nullptr == ui->tableWidget->item(index,2))
            {
                ui->tableWidget->setItem(index,2,new QTableWidgetItem((QString::number(current_distance.toDouble(),'f',3))));
            }
            else
            {
                //单元格中没有内容
                if(ui->tableWidget->item(index,2)->text().isEmpty())
                {
                    ui->tableWidget->item(index,2)->setText(QString::number(current_distance.toDouble(),'f',3));
                }
            }

            if(nullptr == ui->tableWidget->item(index,3))
            {
                ui->tableWidget->setItem(index,3,new QTableWidgetItem(QString::number(pileNum.toDouble(),'f',3)));
            }
            else
            {
                //单元格中没有内容
                if(ui->tableWidget->item(index,3)->text().isEmpty())
                {
                    ui->tableWidget->item(index,3)->setText(QString::number(pileNum.toDouble(),'f',3));
                }
            }

            break;
        }
    }

    int colCount = ui->tableWidget->columnCount();
    for (int row = 0; row < rowCount; row++)
    {
        for(int col = 0; col < colCount; col++)
        {
            QTableWidgetItem *item = ui->tableWidget->item(row, col);
            if (item) {
                item->setTextAlignment(Qt::AlignCenter);
            }
        }
    }
}

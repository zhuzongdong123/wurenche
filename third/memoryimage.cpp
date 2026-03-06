#include "memoryimage.h"
#include <QDebug>
#include <QPainter>
#include <QApplication>
#include "appcommonbase.h"
#include "appeventbase.h"
#include "memoryimage.h"
#include <QKeyEvent>
#include <QIntValidator>
MemoryImage::MemoryImage(QWidget *parent):QLabel(parent)
{
    m_lablePic = new QLabel(this);
    m_lablePic->setScaledContents(true);
    connect(&m_HikCamera, &HikCamera::sendImageSize, this, &MemoryImage::slt_setImageSize);
    this->installEventFilter(this);
    setScaledContents(true);

    //相机参数调节弹窗
    m_cameraParams = new CameraParamsSetting(this);
    connect(m_cameraParams, &CameraParamsSetting::sig_exposureTimeValueChanged, &m_HikCamera, &HikCamera::setExposureTime);
    connect(m_cameraParams, &CameraParamsSetting::sig_gainValueChanged, &m_HikCamera, &HikCamera::setgainValue);
    connect(m_cameraParams, &CameraParamsSetting::sig_exposureTimeValueChanged, this, &MemoryImage::sig_exposureTimeValueChanged);
    connect(m_cameraParams, &CameraParamsSetting::sig_gainValueChanged, this, &MemoryImage::sig_gainValueChanged);
    connect(m_cameraParams, &CameraParamsSetting::sig_gammaValueChanged, &m_HikCamera, &HikCamera::setgammaValue);
    connect(&m_HikCamera, &HikCamera::sendParams, m_cameraParams, &CameraParamsSetting::slt_setParams);
    connect(&m_HikCamera, &HikCamera::exposureTimeChanged, m_cameraParams, &CameraParamsSetting::slt_exposureTimeChanged);
    m_cameraParams->setVisible(false);
}
MemoryImage::~MemoryImage()
{

}

void MemoryImage::openCamera(int index)
{
    qDebug() << QThread::currentThreadId() << "MemoryImage::openCamera QT threadid";
    m_HikCamera.setWinId((HWND)m_lablePic->winId());
    m_HikCamera.setCamera(index);
}

void MemoryImage::restart()
{
    m_HikCamera.startRecord();
}

void MemoryImage::closeCamera()
{
    QTimer::singleShot(2000, [=]() {
        m_HikCamera.freeCamera();
    });
}

void MemoryImage::resetCameraParent(int index)
{
    m_index = index;
    if(CAMERA_TYPE::CL042 == CAMERA_TYPE(index) || CAMERA_TYPE::CL042_2 == CAMERA_TYPE(index))
    {
        m_cameraParams->setParent(this->parentWidget()->parentWidget());
        m_cameraParams->setExposureTimeMaxValue(100);
    }
    else
    {
        m_cameraParams->setParent(this->parentWidget());
    }

    //右相机监听全局的键盘事件
    if(CAMERA_TYPE::CL042_2 == CAMERA_TYPE(index))
    {
        qApp->installEventFilter(this);
    }
}

void MemoryImage::startRecord(QString saveFolderPath)
{
    m_HikCamera.startRecord();
    m_HikCamera.setSaveFolderPath(saveFolderPath);
}

void MemoryImage::stopRecord()
{
    m_HikCamera.stopRecord();
    m_HikCamera.setSaveFolderPath("");
    AppCommonBase::getInstance()->bIsGPSReset = false;
    if(nullptr != m_lablePic)
        m_lablePic->setUpdatesEnabled(true);//停止存储的时候，窗口可以刷新
}

void MemoryImage::openCameraParmas()
{
    QString ddd = m_cameraParams->parentWidget()->objectName();
    //相机参数设置弹窗，点击一次隐藏，再次点击显示。
    m_cameraParams->setVisible(!m_cameraParams->isVisible());

    if(CAMERA_TYPE::CL042 == CAMERA_TYPE(m_index) || CAMERA_TYPE::CL042_2 == CAMERA_TYPE(m_index))
        m_cameraParams->setGeometry(0,m_cameraParams->parentWidget()->height()-m_cameraParams->height(),m_cameraParams->parentWidget()->width(),m_cameraParams->height());
    else
    {
        m_cameraParams->setParent(this);
        m_cameraParams->setGeometry(0,this->height()-m_cameraParams->height(),this->width(),m_cameraParams->height());
    }
}

void MemoryImage::resizeEvent(QResizeEvent *event)
{
    QLabel::resizeEvent(event);
    resetPicLabel();

    if(CAMERA_TYPE::CL042 == CAMERA_TYPE(m_index) || CAMERA_TYPE::CL042_2 == CAMERA_TYPE(m_index))
        m_cameraParams->setGeometry(0,m_cameraParams->parentWidget()->height()-m_cameraParams->height(),m_cameraParams->parentWidget()->width(),m_cameraParams->height());
    else
    {
        m_cameraParams->setParent(this);
        m_cameraParams->setGeometry(0,this->height()-m_cameraParams->height(),this->width(),m_cameraParams->height());
    }
}

void MemoryImage::mouseDoubleClickEvent(QMouseEvent *event)
{
    QLabel::mouseDoubleClickEvent(event);
    QApplication::processEvents();
    if(m_isDouble)
        return;

    emit sig_mouseDoubleClicked();
    m_isDouble = true;
    QTimer::singleShot(300, [=]() {
        m_isDouble = false;
    });
}

bool MemoryImage::eventFilter(QObject *watched, QEvent *event)
{
//    //调节窗口显示的时候，键盘的上下键，调节上下的边距
//    if(event->type() == QEvent::KeyPress  && m_cameraParams != nullptr && m_cameraParams->isVisible())
//    {
//        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
//        if (keyEvent->key() == Qt::Key_Up) {
//            emit sig_keyUp();
//            return true; // 事件已处理
//        } else if (keyEvent->key() == Qt::Key_Down) {
//            emit sig_keyDown();
//            return true; // 事件已处理
//        }
//    }
    return QLabel::eventFilter(watched, event);
}

void MemoryImage::imageReady(const QImage &image, int width, int height)
{
    m_SrcWidth = width;
    m_SrcHeight = height;
    m_image = image;
    update();
}

void MemoryImage::slt_setImageSize(int width, int height)
{
    if(m_SrcWidth != width || m_SrcHeight != height)
    {
        m_SrcWidth = width;
        m_SrcHeight = height;

        //已图像的高度为准
        resetPicLabel();

        if(nullptr != m_lablePic)
            m_lablePic->setUpdatesEnabled(false);//有图像的时候窗口不刷新
    }
}

void MemoryImage::resetPicLabel()
{
    if(0 == m_SrcWidth)
        return;

    //已图像的高度为准
    double radius1 = double(this->height())/this->width();
    double radisu2 = double(m_SrcHeight)/m_SrcWidth;
    if(radius1 < radisu2)
    {
        int height = this->height();
        double radius = double(m_SrcHeight)/m_SrcWidth;
        int width = height/radius;

        if(m_lablePic->width() != width || m_lablePic->height() != height)
        {
            m_lablePic->setFixedSize(width,height);
            m_lablePic->setGeometry((this->width()-width)/2,0,width,height);
        }
    }
    else
    {
        int width = this->width();
        double radius = double(m_SrcHeight)/m_SrcWidth;
        int height = width*radius;

        if(m_lablePic->width() != width || m_lablePic->height() != height)
        {
            m_lablePic->setFixedSize(width,height);
            m_lablePic->setGeometry(0,(this->height()-height)/2,width,height);
        }
    }
}

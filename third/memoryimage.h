#ifndef MEMORYIMAGE_H
#define MEMORYIMAGE_H

#include <QQuickImageProvider>
#include <QPixmap>
#include "hikcamera.h"
#include <QLabel>
#include "cameraparamssetting.h"
#include <QLineEdit>
//extern "C"
//{
//#include <libavcodec/avcodec.h>
//#include <libavformat/avformat.h>
//#include <libavutil/pixfmt.h>
//#include <libswscale/swscale.h>
//#include <libavdevice/avdevice.h>
//#include <libavutil/pixdesc.h>
//#include <libavutil/hwcontext.h>
//#include <libavutil/opt.h>
//#include <libavutil/avassert.h>
//#include <libavutil/imgutils.h>
//#include <libavutil/mem.h>
//}
//#define HIKCAMERA_FMT   AV_PIX_FMT_BAYER_RGGB8

class MemoryImage : public QLabel
{
    Q_OBJECT
public:
    explicit MemoryImage(QWidget *parent = nullptr);
    ~MemoryImage();

    //打开相机
    void openCamera(int index);
    void restart();
    void closeCamera();

    void resetCameraParent(int index);

    void startRecord(QString saveFolderPath);
    void stopRecord();

    void openCameraParmas();

    HikCamera& getHikCamera(){return m_HikCamera;}
protected:
    void resizeEvent(QResizeEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);

public slots:
    void imageReady(const QImage& image, int width, int height);
    void slt_setImageSize(int width, int height);

signals:
    void sig_mouseDoubleClicked();
    void sig_exposureTimeValueChanged(int value);
    void sig_gainValueChanged(double value);
    void sig_keyUp();
    void sig_keyDown();
    void sig_offsetYChanged(QString);

private:
    bool m_isInit = false;
    QImage m_image;
    QPixmap m_pixmap;
    bool m_isDouble = false;
    HikCamera m_HikCamera;
    CameraParamsSetting* m_cameraParams;
    int m_SrcWidth = 0;//图像的宽度
    int m_SrcHeight = 0;//图像的高度
    QLabel* m_lablePic = nullptr;
    void resetPicLabel();
    int m_index = 1;
    //QLineEdit* m_edit = nullptr;
};

#endif // MEMORYIMAGE_H

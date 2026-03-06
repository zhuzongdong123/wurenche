#ifndef CONVERTIMAGE_H
#define CONVERTIMAGE_H

#include <QObject>
#include <QImage>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
#include <libavutil/pixdesc.h>
#include <libavutil/hwcontext.h>
#include <libavutil/opt.h>
#include <libavutil/avassert.h>
#include <libavutil/imgutils.h>
#include <libavutil/mem.h>
}
#define HIKCAMERA_FMT   AV_PIX_FMT_BAYER_RGGB8

class ConvertImage : public QObject
{
    Q_OBJECT
public:
    explicit ConvertImage(QObject *parent = nullptr);
    ~ConvertImage();
    void setWindowSize(int width, int height);

signals:
    void imageReady(const QImage& image,int width, int height);

private:
    SwsContext* pSwsCtx = NULL;
    AVFrame* pSrcFrame = NULL;
    AVFrame* pDstFrame = NULL;
    uint8_t* pDstBuffer = NULL;
    int m_SrcWidth = 0;//图像的宽度
    int m_SrcHeight = 0;//图像的高度
    int m_DstWidth = 0;
    int m_DstHeight = 0;

    int m_windowWidth = 0;
    int m_windowHeight = 0;
    bool m_isRunning = false;

public slots:
    void onRawDataGot(unsigned char *data, int width, int height);

private:
    void initFFMpeg();
    QImage convertFrame(unsigned char *data);
};

#endif // CONVERTIMAGE_H

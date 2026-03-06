#include "convertimage.h"
#include <QDebug>

ConvertImage::ConvertImage(QObject *parent) : QObject(parent)
{

}

ConvertImage::~ConvertImage()
{
    if(this->pSwsCtx)
        sws_freeContext(this->pSwsCtx);
    if(this->pSrcFrame)
        av_free(this->pSrcFrame);
    if(this->pDstFrame)
        av_free(this->pDstFrame);
    if(this->pDstBuffer)
        av_free(this->pDstBuffer);
}

void ConvertImage::setWindowSize(int width, int height)
{
    m_windowWidth = width;
    m_windowHeight = height;
}

void ConvertImage::onRawDataGot(unsigned char *data, int width, int height)
{
    if(m_isRunning)
    {
        qDebug() << "异常信息---解码中，UI中跳过本图片";
        return;
    }

    m_isRunning = true;
    if((this->m_DstWidth != m_windowWidth || this->m_DstHeight != m_windowHeight ||
            this->m_SrcWidth != width || this->m_SrcHeight != height || this->pSwsCtx == NULL || this->pSrcFrame == NULL))
    {
        this->m_DstWidth = m_windowWidth;
        this->m_DstHeight = m_windowHeight;
        this->m_SrcWidth = width;
        this->m_SrcHeight = height;
        this->initFFMpeg();
    }
    convertFrame(data);
    m_isRunning = false;
}

void ConvertImage::initFFMpeg()
{
    int size;

    if(this->m_DstWidth == 0 || this->m_DstHeight == 0)
    {
        qCritical("init FFMpeg but scale size is not set");
        return;
    }
    if(this->pSwsCtx)
        sws_freeContext(this->pSwsCtx);
    if(this->pSrcFrame)
        av_free(this->pSrcFrame);
    if(this->pDstFrame)
        av_free(this->pDstFrame);
    if(this->pDstBuffer)
        av_free(this->pDstBuffer);

    //pSwsCtx
    this->pSwsCtx = sws_alloc_context();
    this->pSwsCtx = sws_getCachedContext(this->pSwsCtx,
                    this->m_SrcWidth, this->m_SrcHeight, HIKCAMERA_FMT,
                    this->m_DstWidth, this->m_DstHeight, AV_PIX_FMT_RGB32,
                    SWS_FAST_BILINEAR, NULL, NULL, NULL);
    //SWS_BICUBIC
    if (this->pSwsCtx == NULL)
    {
        qCritical() << "sws_getContext fail";
        return;
    }

    //pSrcFrame
    this->pSrcFrame = av_frame_alloc();
    if (this->pSrcFrame == NULL)
    {
        qCritical() << "av_frame_alloc pSrcFrame fail";
        return;
    }
    this->pSrcFrame->width = this->m_SrcWidth;
    this->pSrcFrame->height = this->m_SrcHeight;
    this->pSrcFrame->format = HIKCAMERA_FMT;

    //pDstFrame
    this->pDstFrame = av_frame_alloc();
    this->pDstFrame->width = this->m_DstWidth;
    this->pDstFrame->height = this->m_DstHeight;
    this->pDstFrame->format = AVPixelFormat(AV_PIX_FMT_RGB32);
    if (this->pDstFrame == NULL)
    {
        qCritical() << "av_frame_alloc pDstFrame fail";
        return;
    }
    size = av_image_get_buffer_size(AVPixelFormat(AV_PIX_FMT_RGB32), this->m_DstWidth, this->m_DstHeight, 1);
    this->pDstBuffer = (uint8_t*)(av_malloc(size));

    av_image_fill_arrays(this->pDstFrame->data, this->pDstFrame->linesize, this->pDstBuffer,
                         AVPixelFormat(AV_PIX_FMT_RGB32), this->m_DstWidth, this->m_DstHeight, 1);
    // qDebug("dst: %dx%d fmt:%x", this->pDstFrame->width, this->pDstFrame->height, this->pDstFrame->format);
}
QImage ConvertImage::convertFrame(unsigned char *data)
{
    int err = 0;

    if(this->pSrcFrame == NULL || this->pDstFrame == NULL || this->pSwsCtx == NULL)
        return QImage("");

    av_image_fill_arrays(this->pSrcFrame->data, this->pSrcFrame->linesize, data, AVPixelFormat(HIKCAMERA_FMT),
                         this->m_SrcWidth, this->m_SrcHeight, 1);

    err = sws_scale(this->pSwsCtx, (uint8_t const* const*)this->pSrcFrame->data, this->pSrcFrame->linesize, 0,
                    this->pSrcFrame->height, this->pDstFrame->data, this->pDstFrame->linesize);
    if (err < 0)
    {
        qCritical() << "sws_scale error code " << err;
        return QImage("");
    }

    // 设置QImage
    QImage image = QImage((uint8_t *)this->pDstFrame->data[0], this->pDstFrame->width, this->pDstFrame->height,
                    QImage::Format_RGB32);

    QImage image_copy = image.copy(image.rect());
    emit imageReady(image_copy, m_SrcWidth, m_SrcHeight);
    return image;
}

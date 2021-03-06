#include "imageacquisition.h"
#include <QDebug>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QCoreApplication>
#include "defaults.h"
#include <exception>
#include <usoundutils.h>


// Setters and Getters
QString ImageAcquisition::getDeviceName() const
{
    return deviceName;
}

void ImageAcquisition::setDeviceName(const QString &value)
{
    deviceName = value;
}

int ImageAcquisition::getCounter() const
{
    return counter;
}

void ImageAcquisition::setCounter(int value)
{
    counter = value;
}

HalconCpp::HFramegrabber ImageAcquisition::getImageAcquisitionHandle() const
{
    return imageAcquisitionHandle;
}

void ImageAcquisition::setImageAcquisitionHandle(const HalconCpp::HFramegrabber &value)
{
    imageAcquisitionHandle = value;
}

bool ImageAcquisition::getStopAcquisition() const
{
    return stopAcquisition;
}

void ImageAcquisition::setStopAcquisition(bool value)
{
    stopAcquisition = value;
}

void ImageAcquisition::startAquisition(){
    ImageAcquisition::run();
}

bool ImageAcquisition::getSupplyHistogramData() const
{
    return supplyHistogramData;
}

void ImageAcquisition::setSupplyHistogramData(bool value)
{
    supplyHistogramData = value;
}

QString ImageAcquisition::getDeviceType() const
{
    return deviceType;
}

void ImageAcquisition::setDeviceType(const QString &value)
{
    deviceType = value;
}


QString ImageAcquisition::getDeviceMake() const
{
    return deviceMake;
}

void ImageAcquisition::setDeviceMake(const QString &value)
{
    deviceMake = value;
}


// Constructor
bool ImageAcquisition::getSupplyHistogramDataForLine() const
{
    return supplyHistogramDataForLine;
}

void ImageAcquisition::setSupplyHistogramDataForLine(bool value)
{
    supplyHistogramDataForLine = value;
}

ImageAcquisition::ImageAcquisition(QString deviceType,QString deviceMake, QString deviceName, QObject *parent): QThread(parent)
{
    HalconCpp::HFramegrabber imageAcquisitionHandle(HalconCpp::HString(deviceType.toUtf8().constData()).Text(),
                                                    0, 0, 0, 0, 0, 0,
                                                    "progressive", -1, "default",
                                                    -1, "false", "default",
                                                    HalconCpp::HString(
                                                        deviceName.toUtf8().constData()).Text(), 0, -1);
    this->imageAcquisitionHandle = imageAcquisitionHandle;
    this->deviceName = deviceName;
    this->deviceType = deviceType;
    this->deviceMake = deviceMake;
}

QList<QPair<int, int> > ImageAcquisition::getCoordinates() const
{
    return coordinates;
}

void ImageAcquisition::setCoordinates(const QList<QPair<int, int> > &value)
{
    coordinates = value;
}


void ImageAcquisition::run()
{
    QImage qImage;
    using namespace HalconCpp;
    Hlong  width,height;
    QList<QLineSeries*> absoluteHistFrequencies;
    QList<QString> colors = {"red","green","blue"};
    this->imageAcquisitionHandle.GrabImageStart(0);


    try {

        while(!stopAcquisition)
        {
            currentImage = this->imageAcquisitionHandle.GrabImageAsync(0);
            currentImage = currentImage.ZoomImageSize(IMAGE_CONFIGURATION::IMAGE_RESOLUTION_WIDTH, IMAGE_CONFIGURATION::IMAGE_RESOLUTION_HEIGHT, "constant");
            width = currentImage.Width(); height = currentImage.Height();
            if (imageRotation > 0.0){
                currentImage = currentImage.RotateImage(imageRotation, "constant");
            }

            if (mirrorImageHorizontal){
                currentImage = currentImage.MirrorImage("column");
            }

            if (mirrorImageVertical){
                currentImage = currentImage.MirrorImage("row");
            }


            if (enableGrid){
                HalconCpp::HRegion *grid = new HalconCpp::HRegion();
                HalconCpp::HTuple *tuple= new HalconCpp::HTuple;
                tuple->Append(255.0);
                tuple->Append(255.0);
                tuple->Append(255.0);
                HalconCpp::HString *str =  new HalconCpp::HString("fill");
                HalconCpp::GenGridRegion(grid, IMAGE_CONFIGURATION::IMAGE_GRID_ROWS, IMAGE_CONFIGURATION::IMAGE_GRID_COLUMNS, "lines", currentImage.Width(), currentImage.Height());
                currentImage = currentImage.PaintRegion(*grid, *tuple, *str);
            }

            auto conversionStatus = HImage2QImage(currentImage, qImage);

            if (!conversionStatus)
            {
                // failed to convert himage to qimage. Handle it here
                QCoreApplication::quit();
            }

            emit renderImageSignal(qImage);

            if(getRecording()){
                imageBuffer.enqueue(RecordingBuffer(currentImage, currentRecordSaveDir+QString::number(currentBufferImageCounter)+"."+IMAGE_CONFIGURATION::IMAGE_FORMAT));
                emit updateStatusBarSignal(QString("Images in buffer %1").arg(currentBufferImageCounter));
                currentBufferImageCounter+=1;
            }
            if(getSupplyHistogramData()){
                int max=0;
                int channels=currentImage.CountChannels().L();
                if(channels == 3)
                {
                    for(int c=1;c<=channels;c++)
                    {
                        QLineSeries *series = new QLineSeries();
                        HTuple relativeHisto, absoluteHisto;
                        HalconCpp::GrayHisto(HalconCpp::HRegion(0.0,0.0,double(width-1), double(height-1)), currentImage.AccessChannel(c), &absoluteHisto,&relativeHisto);
                        for(int l=0; l<absoluteHisto.Length()-1;l++)
                        {
                            long currentVal = absoluteHisto[l].L();
                            if(max<currentVal)
                                max = currentVal;
                            series->append(l, currentVal);
                        }
                        QPen pen = series->pen();
                        pen.setBrush(QBrush(colors.at(c-1).toUtf8().constData()));
                        series->setPen(pen);
                        absoluteHistFrequencies.append(series);
                    }

                }
                else{
                    QLineSeries *series = new QLineSeries();
                    HTuple relativeHisto, absoluteHisto;
                    HalconCpp::GrayHisto(HalconCpp::HRegion(0.0,0.0,double(width-1), double(height-1)), currentImage, &absoluteHisto,&relativeHisto);
                    for(int l=0; l<absoluteHisto.Length()-1;l++)
                    {
                        long currentVal = absoluteHisto[l].L();
                        if(max<currentVal)
                            max = currentVal;
                        series->append(l, currentVal);
                    }
                    absoluteHistFrequencies.append(series);
                }

                emit renderHistogramSignal(absoluteHistFrequencies, max);
                absoluteHistFrequencies.clear();
            }
            if(getSupplyHistogramDataForLine())
            {
                // Data for this histogram is calculated in ImageStreamWindow.cpp createHistogramWindow function
                int channels=currentImage.CountChannels().L();
                if(channels == 3)
                {
                    for(int c=1;c<=channels;c++)
                    {
                        QLineSeries *series = new QLineSeries();
                        QPen pen = series->pen();
                        pen.setBrush(QBrush(colors.at(c-1).toUtf8().constData()));
                        series->setPen(pen);
                        absoluteHistFrequencies.append(series);
                    }
                    for(int l=0; l<coordinates.size();l++)
                    {
                        int row = coordinates.at(l).second;
                        int column = coordinates.at(l).first;
                        HalconCpp::HTuple pixelIntensity = currentImage.GetGrayval(row,column);
                        absoluteHistFrequencies.at(0)->append(l,pixelIntensity.LArr()[0]);
                        absoluteHistFrequencies.at(1)->append(l,pixelIntensity.LArr()[1]);
                        absoluteHistFrequencies.at(2)->append(l,pixelIntensity.LArr()[2]);
                    }
                }
                else{
                    QLineSeries *series = new QLineSeries();
                    for(int l=0; l<coordinates.size();l++)
                    {
                        int pixelIntensity = currentImage.GetGrayval(coordinates.at(l).second, coordinates.at(l).first);
                        series->append(l, pixelIntensity);
                    }
                    absoluteHistFrequencies.append(series);
                }

                emit renderHistogramSignalForLine(absoluteHistFrequencies, coordinates.size());
                absoluteHistFrequencies.clear();
            }
            counter++;
        }        

    } catch (HalconCpp::HOperatorException &e) {
        qDebug() << e.ErrorMessage().Text();
    }
     catch (HalconCpp::HException &e) {
        qDebug() << e.ErrorMessage().Text();
    }
    catch (std::exception &e) {
        qDebug() << e.what();
    }




}




//*
// * @brief HImage2QImage Converts Halcon's HImage to Qt's QImage
//       * @param from HImage , currently only supports 8bits grayscale image and 8bits 3 channel color image
//       * @param to QImage , where from and to do not share memory. If the memory size of to is appropriate, then there is no need to redistribute memory. So you can speed up.
//       * @return true means the conversion was successful, false means the conversion failed

bool ImageAcquisition::HImage2QImage(HalconCpp::HImage &from, QImage &to)
{
    using namespace HalconCpp;
    Hlong width;
    Hlong height;
    from.GetImageSize(&width, &height);

    HTuple channels = from.CountChannels();
    HTuple type = from.GetImageType();

    if( strcmp(type[0].S(), "byte" )) // if it is not a byte type, it fails
    {
        return false;
    }
    QImage::Format format;
    switch(channels[0].I())
    {
    case 1:
        format = QImage::Format_Grayscale8;
        break;
    case 3:
        format = QImage::Format_RGB32;
        break;
    default:
        return false;
    }

    if(to.width() != width || to.height() != height || to.format() != format)
    {
        to = QImage(static_cast<int>(width),
                    static_cast<int>(height),
                    format);
    }
    HString Type;
    if(channels[0].I() == 1)
    {
        unsigned char * pSrc = reinterpret_cast<unsigned char *>( from.GetImagePointer1(&Type, &width, &height) );
        memcpy( to.bits(), pSrc, static_cast<size_t>(width) * static_cast<size_t>(height) );
        return true;
    }
    else if(channels[0].I() == 3)
    {
        uchar *R, *G, *B;
        from.GetImagePointer3(reinterpret_cast<void **>(&R),
                              reinterpret_cast<void **>(&G),
                              reinterpret_cast<void **>(&B), &Type, &width, &height);

        for(int row = 0; row < height; row ++)
        {
            QRgb* line = reinterpret_cast<QRgb*>(to.scanLine(row));
            for(int col = 0; col < width; col ++)
            {
                line[col] = qRgb(*R++, *G++, *B++);
            }
        }
        return true;
    }

    return false;
}

HalconCpp::HTuple ImageAcquisition::getValueForParam(std::string paramString)
{
    HalconCpp::HTuple paramValue;
    try {
        qDebug() << "Param string" << paramString.c_str();
        paramValue = imageAcquisitionHandle.GetFramegrabberParam(paramString.c_str());
    } catch (std::exception &e) {
        qDebug() << e.what();
    }
    return paramValue;

}

//void ImageAcquisition::setValueForParam(std::string paramString, int paramValue)
//{
//    try {
//        qDebug() << "in int set param"<< paramString.c_str() << paramValue;
//        imageAcquisitionHandle.SetFramegrabberParam(paramString.c_str(), paramValue);

//    } catch (HalconCpp::HException &e) {
//        qDebug() << "Exception in setting param value:int message"<<e.ErrorMessage().Text() << e.ErrorCode();
//    }
//}

void ImageAcquisition::setValueForParam(std::string paramString, double paramValue)
{
    try {
        qDebug() << "double set param"<< paramString.c_str() << paramValue;
        imageAcquisitionHandle.SetFramegrabberParam(paramString.c_str(), paramValue);

    } catch (HalconCpp::HException &e) {
        qDebug() << "Exception in setting param value:double message"<<e.ErrorMessage().Text() << e.ErrorCode();
    }
}

void ImageAcquisition::setValueForParam(std::string paramString, bool paramState)
{
    try {
        qDebug() << "bool set param"<< paramString.c_str() << paramState;
        imageAcquisitionHandle.SetFramegrabberParam(paramString.c_str(), paramState);

    } catch (HalconCpp::HException &e) {
        qDebug() << "Exception in setting param value:double message"<<e.ErrorMessage().Text() << e.ErrorCode();
    }
}

//void ImageAcquisition::setValueForParam(std::string paramString, long paramValue)
//{
//    try {
//        qDebug() << "long set param"<< paramString.c_str() << paramValue;
//        imageAcquisitionHandle.SetFramegrabberParam(paramString.c_str(), (Hlong)paramValue);

//    } catch (HalconCpp::HException &e) {
//        qDebug() << "Exception in setting param value:double message"<<e.ErrorMessage().Text() << e.ErrorCode();
//    }
//}

void ImageAcquisition::setValueForParam(std::string paramString, std::string paramValue)
{
    try {
        qDebug() << "string set param"<< paramString.c_str() << paramValue.c_str();
        imageAcquisitionHandle.SetFramegrabberParam(paramString.c_str(), paramValue.c_str());

    }  catch (HalconCpp::HException &e) {
        qDebug() << "Exception in setting param value:string message"<<e.ErrorMessage().Text() << e.ErrorCode();
    }
}




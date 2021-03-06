#ifndef IMAGESTREAMWINDOW_H
#define IMAGESTREAMWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QTreeWidgetItem>
#include "imageacquisition.h"
#include <QMap>
#include <QLabel>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include<QPixmap>
#include <QGraphicsView>
#include <QSpinBox>
#include <QCheckBox>
#include "spinboxcontainer.h"
#include "parametercontainer.h"
#include "histogramwindow.h"
#include <QtCharts/QChartView>
#include <QPushButton>
#include <customscene.h>
//using namespace QtCharts;


class ImageStreamWindow : public QMainWindow
{
    Q_OBJECT

private:
    ImageAcquisition *imageAcquisitionThread;

public:
    explicit ImageStreamWindow(ImageAcquisition* imageAcquisitionThread, QWidget *parent = nullptr);
    QDir qDir;

    // widgets
    QTreeWidgetItem *exposureControls;
    QTreeWidgetItem *exposureTime;
    QSpinBox *exposureTimeSpinBox;
    QTreeWidgetItem *analogGain;
    QDoubleSpinBox *analogGainSpinBox;
    QTreeWidgetItem *autoGain;
    QCheckBox *autoGainCheckbox;
    QTreeWidgetItem *autoExposure;
    QCheckBox *autoExposureCheckbox;
    QTreeWidgetItem *colorAppearance;

    QTreeWidgetItem *hue;
    QSpinBox *hueSpinBox;
    QTreeWidgetItem *saturation;
    QSpinBox *saturationSpinBox;
    QTreeWidgetItem *brightness;
    QSpinBox *brightnessSpinBox;
    QTreeWidgetItem *contrast;
    QSpinBox *contrastSpinBox;
    QTreeWidgetItem *gamma;
    QDoubleSpinBox *gammaSpinBox;
    QTreeWidgetItem *acquisitionFrameRate;
    QSpinBox *acquisitionFramerateSpinBox;
    QTreeWidgetItem *acquisitionFrameRateEnable;
    QCheckBox *acquisitionFrameRateEnableCheckbox;
    QTreeWidgetItem *monochrome;
    QCheckBox *monochromeCheckbox;
    QTreeWidgetItem *rgb;
    QCheckBox *rgbCheckbox;
    QToolBar* toolBar;

    QLabel *imageLabel;

    QGraphicsView *graphicsView;
    CustomScene *graphicsScene;
    QGraphicsPixmapItem *graphicsPixmapItem;
    QActionGroup *actionGroup;

    QList<QGraphicsPixmapItem *> graphicsPixmapItemList;
    QList<ParameterContainer *> containers;
    CameraParameters cameraParameters;


    // Actions
    QAction *imageSaveButton, *recordButton, *recordPauseButton, *recordStopButton, *grayHistogramButton, *fixedAspectRatioButton,
    *rotateClockwise90Button, *rotateAntiClockwise90Button, *mirrorImageHorizontalButton, *mirrorImageVerticalButton, *resetImageButton,
    *gridButton, *selectButton, *lineButton;
    Qt::AspectRatioMode fixedAspectRatio=Qt::KeepAspectRatio;        

    // Chart window
    HistogramWindow *histogramWindow;
    QChartView *chartView;



    // member functions
    void setupCameraWindow();


    // setters and getters
    HalconCpp::HFramegrabber getAcquisitionHandle() const;
    void setAcquisitionHandle(const HalconCpp::HFramegrabber &value);
    ImageAcquisition *getImageAcquisitionThread() const;
    void setImageAcquisitionThread(ImageAcquisition *value);

    CameraParameters getCameraParameters() const;
    void setCameraParameters(const CameraParameters &value);
    void restoreDeviceSpecificSettings();
    void createToolBar();
    void addToContainer(ParameterContainer *container, bool exists);

public slots:
    void renderImage(QImage qImage);
    void updateStatusBar(QString statusMsg);
    void saveImage();
    void startVideoRecord();
    void pauseVideoRecord();
    void stopVideoRecord();
    void setFixedAspectRatio();    
    void rotateAntiClockwise90Deg();
    void mirrorImageHorizontalSlot();
    void mirrorImageVerticalSlot();
    void gridSlot();
    void resetImageSlot();
    void selectSlot();
    void lineSlot();
    void actionGroupClicked(QAction*);
    void writeQueue();
    void updateAllParameters();
    void createHistogramWindow();
    void renderHistogramSlot(QList<QLineSeries*>, int);
    void renderHistogramSlotForLine(QList<QLineSeries*>, int);

signals:


protected:
    void closeEvent(QCloseEvent *event);
};

#endif // IMAGESTREAMWINDOW_H

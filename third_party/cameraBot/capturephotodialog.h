#ifndef CAPTUREPHOTODIALOG_H
#define CAPTUREPHOTODIALOG_H

#include <QDialog>
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include <QTimer>
#include <QFrame>
#include <QImage>
#include <QVBoxLayout>
#include <QPushButton>
#include <QComboBox>


//namespace Ui {
//    class CapturePhotoDialog;
//}

class CapturePhotoDialog : public QDialog {
    Q_OBJECT

    public:
        CapturePhotoDialog(QWidget *parent = 0,QString argFilename="photos/captured.jpg",int argLinesVisibility=2);
        ~CapturePhotoDialog();
        QStringList availableCameraDevices;


    private:
        QImage io_image;        //handle for raw data from camera interface
        QFrame *viewFinder;     //Where the image is displayed on the dialog
        QTimer *refresh_rate;   //Update the frame
        QTimer *snap_refresh_rate;   //Set focus
        QTimer *experimentTiming;
        QVBoxLayout *layout;
        QPushButton *snap;
        QPushButton *switchCamera;
        QSlider *brightnessSlider;


        bool still_painting;    //Hack to stop capture when dialog is closed

        //OpenCV Types
        CvCapture* capture;     //Pointer to video stream
        IplImage* cv_image;     //OpenCV image struct

        void paintEvent(QPaintEvent*);
        void detectCameras();
        bool cameraIsAccessible(int);



    public slots:
        void queryFrame();
        void saveImage();
        void snapPicture();
        void experimentClock();
        void switchCameraDevice();

    private slots:
        void brightnessChanged(int);
        IplImage* cropImage(const IplImage *, const CvRect );
        void keyPressEvent(QKeyEvent * e);
        void closeEvent(QCloseEvent *ev);
        void reject();
        void endCaptureProcess();

    };

#endif // CAPTUREPHOTODIALOG_H

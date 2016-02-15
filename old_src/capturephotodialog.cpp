#include <QtDebug>
#include <QDebug>
#include "capturephotodialog.h"
#include <QPainter>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QProcess>
#include <QMessageBox>
#include <QLabel>
#include <QProcess>
#include <QKeyEvent>

double gradientMultiplier = 100.00; // Valid Range of Brightness is 0.01 to 1.00

IplImage* frame;

//ROI is Region of Interest
int roiX1 = 180;

int defRoiX1 = 180;
int defRoiX2 = 460;
int defRoiY1 = 30;
int defRoiY2 = 400;

int roiX2 = 460;
int roiY1 = 30;
int roiY2 = 400;
int roiThickness = 2;
float aspect_ratio = 0.75;	// 3/4 aspect ratio standard.
bool showLines=true;
int linesVisibility=2;
int activeCamera=-1;
//int *lastActiveCamera=-1;

bool captureStarted = false;
int totalDetectCameras = 0;

int secs = 0;
QString finalFilename;
int frameInterval = 75; //how often should the frame be refreshed in milliseconds
bool cameraFound = false;
int currentCameraIndex = -1;

CapturePhotoDialog::CapturePhotoDialog(QWidget *parent,QString argFilename,int  argLinesVisibility) :
        QDialog(parent)
{

    //SETUP I/O INTERFACE
    detectCameras();
    //    qDebug() << "Cams: " <<totalDetectCameras;
    if(totalDetectCameras<=0){

        QMessageBox::critical(this,"No Camera Device","No Camera Detected. If this problem persists, please shutdown and restart this computer");


    }



    finalFilename = argFilename;
    if(argLinesVisibility==0){
        showLines = false;
    }else{

    }



    linesVisibility = argLinesVisibility;

    // qDebug() << "Target Filename: " << finalFilename;


    QFile cameraChecker;

    showLines = true;

    int selectedCamera = 0;

    //activeCamera = lastActiveCamera;

    if(totalDetectCameras>0){

        switchCameraDevice();

        if(activeCamera > -1) {

            selectedCamera  = activeCamera;
            cameraFound = true;

        } else {

            cameraFound = false;
        }

        if(cameraFound==true) {

            captureStarted = true;

            capture = cvCreateCameraCapture(selectedCamera);


            //cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_WIDTH, 1280 );

            //cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_HEIGHT, 960 );

            IplImage* frame=cvQueryFrame(capture);

            QImage dummy(QSize(frame->width,frame->height),QImage::Format_RGB888);
            io_image = dummy;

            //Share image buffer between QImage and OpenCV IplImage (like unix hard links)
            cv_image = cvCreateImageHeader(cvSize(io_image.width(),io_image.height()),8,3);
            cv_image->imageData = (char*)io_image.bits();

            //Check current frame and detect if to be rotated/fliped
            if (frame->origin == IPL_ORIGIN_TL)
                cvCopy (frame, cv_image, 0);
            else
                cvFlip(frame, cv_image, 0);

            //Raw image data is in BGR colorspace but we need to display in RGB colorspace
            cvCvtColor(cv_image,cv_image,CV_BGR2RGB);

            //Appropriate dimentions if webcam is landscape. We want image area to be centre

            if (io_image.width() > io_image.height()) {

                int new_width = aspect_ratio * io_image.height();

                //get starting X1 point to ensure that desired image boundary is at centre of video
                roiX1 = (( io_image.width() - new_width ) * 0.5)+20;
                roiX2 = roiX1 + new_width-20;
                roiY1 = 30;
                roiY2 = io_image.height()-65;
                //MADE THE BOX SMALLER ON IFES RECOMMENDATION
            }
            else{
                qDebug() << "Seems width is les than height";
                //exit(1); //hopefully wont be used
            }


            //SETUP USER INTERFACE

            viewFinder = new QFrame();
            snap = new QPushButton("Sna&p Picture");

            switchCamera =new QPushButton("&Switch Camera");

            //Dialog box
            this->setWindowTitle("Capture Image");
            this->setMinimumSize(QSize(frame->width,frame->height+75));
            this->setModal(true);

            //Generate the slider to allow adjusting of current brightness settings
            brightnessSlider  = new QSlider;
            brightnessSlider->setGeometry(QRect(80, 260, 160, 19));
            brightnessSlider->setMinimum(0);
            brightnessSlider->setMaximum(100);
            brightnessSlider->setSliderPosition(3);
            brightnessSlider->setOrientation(Qt::Horizontal);
            brightnessSlider->setTickPosition(QSlider::TicksBelow);
            //Restore the slider to match the current brightness value of the camera
            double currentBrightness = cvGetCaptureProperty(capture,CV_CAP_PROP_BRIGHTNESS);
            qDebug()<< "Current Brightness: " <<currentBrightness;
            double translatedBrightness = gradientMultiplier * currentBrightness;
            brightnessSlider->setValue(translatedBrightness);

            connect(brightnessSlider,SIGNAL(valueChanged(int)),this,SLOT(brightnessChanged(int)));

            layout = new QVBoxLayout;

            layout->addWidget(viewFinder);

            layout->addWidget(snap);

            layout->setMargin(0);
            setLayout(layout);
            layout->addWidget(brightnessSlider);


            QLabel *helpText = new QLabel();

            helpText->setGeometry(0,0,0,500);

            helpText->setMaximumHeight(15);

            QString styleSheet;

            styleSheet="color:#ffffff;background-color:#000000;font-weight:bold;font-size:12px;";

            helpText->setStyleSheet(styleSheet);

            helpText->setText("Ensure the Face is in the OVAL. Press the SPACEBAR to Capture or ESC to Cancel Capture");

            layout->addWidget(helpText);

            if(totalDetectCameras>1) {
                layout->addWidget(switchCamera);
                connect(switchCamera,SIGNAL(clicked()),this,SLOT(switchCameraDevice()));
            }


            connect(snap,SIGNAL(clicked()),this,SLOT(saveImage()));




            //SET TIMER INTERVAL -- constantly capture new frames
            refresh_rate = new QTimer();
            connect(refresh_rate,SIGNAL(timeout()),this,SLOT(queryFrame()));
            refresh_rate->start(frameInterval);

            // snap_refresh_rate = new QTimer();
            //connect(snap_refresh_rate,SIGNAL(timeout()),snap,SLOT(setFocus()));
            //snap_refresh_rate->start(700);

            //SET PAINTING CHECK VALUE
            still_painting = true;


            //    experimentTiming = new QTimer();
            //    connect(experimentTiming,SIGNAL(timeout()),this,SLOT(experimentClock()));
            //    experimentTiming->start(1000);
        }else {
            qDebug() << "Got here!";
            //this->close();

        }
    }else{
        qDebug() << "Really no camera";
        viewFinder = new QFrame();
        snap = new QPushButton("Sna&p Picture");

        switchCamera =new QPushButton("&Close");
        connect(switchCamera,SIGNAL(clicked()),this,SLOT(close()));
        //Dialog box
        this->setWindowTitle("Capture Image");
        this->setMinimumSize(640,480);
        this->setModal(true);

        layout = new QVBoxLayout;

        layout->addWidget(viewFinder);





        layout->setMargin(0);
        layout->addWidget(switchCamera);


        setLayout(layout);

        QTimer::singleShot(700,switchCamera,SLOT(click()));


    }
}

void CapturePhotoDialog::queryFrame() {
    //    qDebug() << " capture  " <<capture << " still_painting  " << still_painting;
    if (capture) {

        captureStarted = true;

        frame = cvQueryFrame(capture);
        //assert(frame);
        if(!frame){
            qDebug()<< "Camera crashed -> Frame was invalid ";
            return;
        }



        //Check video condition
        if (frame->origin == IPL_ORIGIN_TL)
            cvCopy (frame, cv_image, 0);
        else
            cvFlip(frame, cv_image, 0);

        //Create a rectangle/box showing the boundary of the photo


        cvRectangle(cv_image,                    /* the dest image */
                    cvPoint(roiX1-roiThickness, roiY1-roiThickness),        /* top left point */
                    cvPoint(roiX2+roiThickness,roiY2+roiThickness ),       /* bottom right point */
                    cvScalar(0, 255, 0, 0), /* the color; green */
                    roiThickness, 1, 0);               /* thickness, line type, shift */
        //cvEllipse(cv_image,cvPoint((roiX2+roiX1)/2),cvPoint((roiY2+roiY1)/2));
        //            cvLine(cv_image,cvPoint(((roiX2+roiX1)/2)-20,roiY1),cvPoint(((roiX2+roiX1)/2)-20,roiY2),cvScalar(0,255,0,0));

        if(showLines){

            //            cvCircle(cv_image,
            //                     cvPoint((roiX1+roiX2)/2, (roiY1+roiY2)/2),
            //                     ((roiX2-roiX1)/2),
            //                     cvScalar(0, 255, 0, 0),
            //                     roiThickness, 1, 0
            //                     );
            if(linesVisibility==2) {
                cvEllipse(cv_image,
                          cvPoint((roiX1+roiX2)/2, (roiY1+roiY2)/2),
                          cvSize((roiX2-roiX1)/2-80, (roiY2-roiY1)/2-60),
                          0,
                          0,
                          360,
                          cvScalar(0, 255, 0, 0),
                          roiThickness, 1, 0
                          );
            }
            cvLine(cv_image, cvPoint(((roiX2+roiX1)/2), roiY1), cvPoint((roiX2+roiX1)/2, roiY2), cvScalar(0,255,0,0));  // Vertical
            cvLine(cv_image, cvPoint(roiX1, (roiY2/2)+20), cvPoint(roiX2, (roiY2/2)+20), cvScalar(0,255,0,0)); // Horizontal
            cvLine(cv_image, cvPoint(roiX1, (roiY2/2)-20), cvPoint(roiX2, (roiY2/2)-20), cvScalar(0,255,0,0)); // Horizontal

            cvLine(cv_image, cvPoint(((roiX2+roiX1)/2)-20, 50), cvPoint(((roiX2+roiX1)/2)+20, 50), cvScalar(0,255,0,0), 1);
            cvLine(cv_image, cvPoint(((roiX2+roiX1)/2)-20, roiY2-25), cvPoint(((roiX2+roiX1)/2)+20, roiY2-25), cvScalar(0,255,0,0), 1);

            cvLine(cv_image, cvPoint(roiX1+20, ((roiY2+roiY1)/2)-20) , cvPoint(roiX1+20, ((roiY2+roiY1)/2)+20), cvScalar(0,255,0,0), 1);
            cvLine(cv_image, cvPoint(roiX2-20, ((roiY2+roiY1)/2)-20) , cvPoint(roiX2-20, ((roiY2+roiY1)/2)+20), cvScalar(0,255,0,0), 1);
        }

        //convert colorspace
        cvCvtColor(cv_image,cv_image,CV_BGR2RGB);

        //Dialog should update itself. This automatically triggers the paint event

        still_painting=false;

        this->update();
    } else {
        qDebug() << "This happened";
        cvReleaseCapture(&capture);
        refresh_rate->stop();
        sleep(0.5);
        this->close();
    }

}

void CapturePhotoDialog::paintEvent(QPaintEvent*) {

    //Repaint frame with image
    QPainter painter(this);
    painter.drawImage(QPoint(viewFinder->x(),viewFinder->y()), io_image);

    //Hack to stop video capture
    //After closing dialog with close button, cam capturing still continues
    still_painting=true;

}

void CapturePhotoDialog::saveImage()
{


    //Saves picture in default location
    QProcess::execute("mkdir photos -p");
    QString delString;
    delString.sprintf("rm -f %s",finalFilename.toUtf8().data());

    QProcess::execute(delString);


    // QString playExe = "mplayer -really-quiet resources/sounds/snap.wav 2>&-";
    //QProcess::startDetached(playExe);

    showLines = false;

    sleep(0.5);       //delay like traditional cameras
    QTimer::singleShot(50,this,SLOT(snapPicture()));

}

void CapturePhotoDialog::snapPicture(){


    cvCvtColor(cv_image,cv_image,CV_BGR2RGB);

    //Crop the image based on the coordinates of the rectangle drawn earlier
    CvRect croppingSize = cvRect( roiX1,roiY1, roiX2-roiX1,roiY2-roiY1 );

    cv_image = cropImage(cv_image,croppingSize);

    //we want to scale-down after the crop operation. Crop operation ensures we have correct aspect ratio

    //after the filename declaration, adjust to this

    const char *filename = finalFilename.toUtf8().data();

    IplImage* scaled_img = cvCreateImage( cvSize(300,(300/aspect_ratio)), IPL_DEPTH_8U, 3 );
    cvResize(cv_image,scaled_img,1);


    cvSaveImage(filename,cv_image);

    //io_image.save(filename); sucks!! don't go near it again

    //Disconnect cam device
    cvReleaseCapture(&capture);

}

CapturePhotoDialog::~CapturePhotoDialog()
{
    cvReleaseCapture(&capture);
}


void CapturePhotoDialog::brightnessChanged(int newBrightness){

    //Adjusts the brightness of the image displayed
    double translatedBrightness = newBrightness/gradientMultiplier;
    //Convert slider value to brightness value
    qDebug() << "Current Brightness Level: " <<cvGetCaptureProperty(capture,CV_CAP_PROP_BRIGHTNESS) << " New Brightness: " << translatedBrightness;
    //Apply New Brightness Value;
    cvSetCaptureProperty(capture,CV_CAP_PROP_BRIGHTNESS,translatedBrightness);

}


IplImage* CapturePhotoDialog::cropImage(const IplImage *img, const CvRect region)
{
    IplImage *imageCropped;
    CvSize size;

    if (img->width <= 0 || img->height <= 0
        || region.width <= 0 || region.height <= 0) {
        qDebug() << "ERROR in cropImage(): invalid dimensions." ;
        exit(1);
    }

    if (img->depth != IPL_DEPTH_8U) {
        qDebug() << "ERROR in cropImage(): image depth is not 8.";
        exit(1);
    }

    // Set the desired region of interest.
    cvSetImageROI((IplImage*)img, region);
    // Copy region of interest into a new iplImage and return it.
    size.width = region.width;
    size.height = region.height;
    imageCropped = cvCreateImage(size, IPL_DEPTH_8U, img->nChannels);
    cvCopy(img, imageCropped);	// Copy just the region.

    return imageCropped;
}

void CapturePhotoDialog::experimentClock() {
    secs+=1;

    qDebug() << secs << " second";
}


void CapturePhotoDialog::switchCameraDevice(){

    if(totalDetectCameras ==0){
        return;
    }

    if(captureStarted==true){
        qDebug() << "Stopping current active camera " << activeCamera;
        refresh_rate->stop();
        cvReleaseCapture(&capture);

    }else{

    }



    try {



qDebug() << "current Camera Index " << currentCameraIndex;



int newCameraIndex = currentCameraIndex + 1;

if(newCameraIndex == 0){
    newCameraIndex = totalDetectCameras - 1;
}

int tentativeActiveCamera = 0;

if(newCameraIndex < availableCameraDevices.size()) {
    tentativeActiveCamera = availableCameraDevices.at(newCameraIndex).toInt();

} else {

    newCameraIndex  = 0;
}
currentCameraIndex = newCameraIndex;
qDebug() << "new  Camera Index " << currentCameraIndex;




        qDebug()<< " availableCameraDevices" << availableCameraDevices;

//        if(tentativeActiveCamera > (totalDetectCameras - 1)){
//            qDebug() << " Looping Camera List ";
//            tentativeActiveCamera = 0; //start again!
//        }
        qDebug() << " Trying New Camera " << tentativeActiveCamera;


        if(availableCameraDevices.contains(QString::number(tentativeActiveCamera).toStdString().c_str())) {
            qDebug()<< " Found this camera " << tentativeActiveCamera;
            if(cameraIsAccessible(tentativeActiveCamera)) {
                qDebug()<< " Can Access this camera " << tentativeActiveCamera;
                activeCamera = tentativeActiveCamera;
            }else{
                qDebug() << "Camera is NOT accessible " << tentativeActiveCamera;
                //                activeCamera = -1;
            }

        } else{
            qDebug()<< " Camera Outside Known Camera List " << tentativeActiveCamera;
            //no show;
        }
    }catch(...){
        qDebug()<< " An Unknown Error Occcured while checking through available cameras ";
        return;

    }
    qDebug() << " Switching to camera " << activeCamera << " captureStarted " << captureStarted;


    if(captureStarted) {
        //        refresh_rate->stop();
        qDebug() << "Trying to switch";

        sleep(3);
        capture = cvCreateCameraCapture(activeCamera);
        frame=cvQueryFrame(capture);




        //still_painting = true;//for the sake of all
        refresh_rate->start(frameInterval);

        snap->setFocus();

    }else{
        qDebug()<< " The capture process has NOT been started yet ";
    }

}

void CapturePhotoDialog::detectCameras(){

    QString deviceName;
    QFile checkCameraFile;
    totalDetectCameras = 0;
    for(int a = 0;a < 7 ; a++){
        deviceName.sprintf("/dev/video%d",a);

        if(checkCameraFile.exists(deviceName)){

            try{
                if(cameraIsAccessible(a)){
                    availableCameraDevices.append(QString::number(a).toStdString().c_str());
                    activeCamera = a - 1;
                    totalDetectCameras++;
                }
            }catch(...) {
                return;
            }
        } else {

            //availableCameraDevices[a] = 0;

        }

    }
    qDebug() << "Detected cameras : " << totalDetectCameras;

}

bool CapturePhotoDialog::cameraIsAccessible(int cameraIndex){

    QProcess *cameraCheckProcess= new QProcess;
    cameraCheckProcess->setProcessChannelMode(QProcess::SeparateChannels);

    QString processStr;
    processStr.sprintf("bin/checkCameraAccess %d 2>&-",cameraIndex);
    qDebug() << processStr;
    cameraCheckProcess->start(processStr);


    if (!cameraCheckProcess->waitForFinished()){
        qDebug() << "Failed :" << cameraCheckProcess->errorString();
        return false;
    } else {
        QString cameraCheckResult =  cameraCheckProcess->readAllStandardOutput();
        qDebug() << "cameraCheck Result: " <<cameraCheckResult;
        if(cameraCheckResult.toInt() == 1){
            return true;
        } else {
            return false;
        }

    }



    return false;
}


void CapturePhotoDialog::keyPressEvent(QKeyEvent *e)
{

    switch (e->key())
    {

    case Qt::Key_S:
        qDebug() << "He pressed switch camera!!!";
        switchCameraDevice();
        break;
    case Qt::Key_Space:
        qDebug() << "He pressed save!!!";
        saveImage();
        break;

        // Default calls the original method to handle standard keys
    default: QWidget::keyPressEvent(e);
    }
}

void CapturePhotoDialog::endCaptureProcess(){

    if(cameraFound==false){
        return;
    }

    refresh_rate->stop();
    cvReleaseCapture(&capture);
    captureStarted = false;

}

void CapturePhotoDialog::closeEvent(QCloseEvent *ev) {

    endCaptureProcess();
    ev->accept();

}

void CapturePhotoDialog::reject() {
    endCaptureProcess();
}

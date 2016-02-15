#include <stdio.h>

#include "/opt/include/opencv/cv.h"
#include "/opt/include/opencv/highgui.h"

int main( int argc, char* argv[] )
{
    CvCapture *capture = 0;
    
    /* initialize camera */
    
	capture = cvCreateCameraCapture(atoi(argv[1]));
    
     /* always check */
    if ( !capture ) {
        printf( "%d", 0);
        return 1;
    }
   
    cvReleaseCapture( &capture );
    printf("%d" ,1);
    return 0;
}

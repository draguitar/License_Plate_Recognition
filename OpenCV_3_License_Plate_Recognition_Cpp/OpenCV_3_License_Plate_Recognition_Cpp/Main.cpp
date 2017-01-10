// Main.cpp

#include "Main.h"
#include <math.h>
#include <climits>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>



using namespace std;
using namespace cv;

//�|�ˤ��J���Ʀ�
double round_val(double src,int n){
    double res;
    int i,k=1;
    for(i=0;i<n;i++)k*=10;
    res=int(src*k+0.5);
    res/=k;
    return res;
} 
bool debug = true ;

int main( int argc, char** argv ){
	if(debug){
		 if( argc != 2)
		{
		 cout <<" Usage: display_image ImageToLoadAndDisplay" << endl;
		 return -1;
		}

		Mat image;
		image = imread(argv[1], CV_LOAD_IMAGE_COLOR);   

		if(! image.data )                              
		{
			cout <<  "Could not open or find the image" << std::endl ;
			return -1;
		}

		namedWindow( "Display window", WINDOW_AUTOSIZE );
		imshow( "Display window", image );     
	}

	//KNN Training
    bool blnKNNTrainingSuccessful = loadKNNDataAndTrainKNN();
    //KNN training���ѮɡA���_�{��
    if (blnKNNTrainingSuccessful == false) {
                                                                        
        std::cout << std::endl << std::endl << "error: KNN traning fail" << std::endl << std::endl;
        return(0);                                                      
    }

    cv::Mat imgOriginalScene;           

    //���J�Ϥ�
    imgOriginalScene = imread(argv[1], CV_LOAD_IMAGE_COLOR);

    if (imgOriginalScene.empty()) {
        std::cout << "error: image not read from file\n\n";
        return(0);                                         
    }
    //
    std::vector<PossiblePlate> vectorOfPossiblePlates = detectPlatesInScene(imgOriginalScene);
    //
    vectorOfPossiblePlates = detectCharsInPlates(vectorOfPossiblePlates);

	if (debug){
		cv::imshow("imgOriginalScene", imgOriginalScene);          
	}
    if (vectorOfPossiblePlates.empty()) {
        std::cout << std::endl << "�䤣�쨮�P !" << std::endl;
    } else {
        //�Y�s�b�i�઺���P�V�q�A�q�̤j���Ʀr-->�̤p���Ʀr�����ƦC
        std::sort(vectorOfPossiblePlates.begin(), vectorOfPossiblePlates.end(), PossiblePlate::sortDescendingByNumberOfChars);

        //���X��char�i�Q���Ѫ�vector�A�]�L���ڭ̭n���o�����P
        PossiblePlate licPlate = vectorOfPossiblePlates.front();

        //show plate & Thresh
        cv::imshow("imgPlate", licPlate.imgPlate);            
        cv::imshow("imgThresh", licPlate.imgThresh);

        //�Y�S���i���Ѫ�char�A���_�{����ܰT��
        if (licPlate.strChars.length() == 0) {
            std::cout << std::endl << "�S�����P�r���i�Q���� !" << std::endl << std::endl;
            return(0);
        }
        //�e�X���P��m
        drawRedRectangleAroundPlate(imgOriginalScene, licPlate);

        //��console����ܿ��ѫ᪺���P
        std::cout << std::endl << "License Plate: " << licPlate.strChars << std::endl;
        std::cout << std::endl << "-----------------------------------------" << std::endl;

        if(debug){
            writeLicensePlateCharsOnImage(imgOriginalScene, licPlate);

            cv::imshow("imgOriginalScene", imgOriginalScene);
            cv::imwrite("imgOriginalScene.png", imgOriginalScene);
        }
    }

    cv::waitKey(0);

    return(0);
}

//�갵�ά��u�إX���P
void drawRedRectangleAroundPlate(cv::Mat &imgOriginalScene, PossiblePlate &licPlate) {
    cv::Point2f p2fRectPoints[4];

    licPlate.rrLocationOfPlateInScene.points(p2fRectPoints);

    for (int i = 0; i < 4; i++) {
        cv::line(imgOriginalScene, p2fRectPoints[i], p2fRectPoints[(i + 1) % 4], SCALAR_RED, 2);
    }
}

//���P�W�g�r
void writeLicensePlateCharsOnImage(cv::Mat &imgOriginalScene, PossiblePlate &licPlate) {
    cv::Point ptCenterOfTextArea;                   // this will be the center of the area the text will be written to
    cv::Point ptLowerLeftTextOrigin;                // this will be the bottom left of the area that the text will be written to

    int intFontFace = CV_FONT_HERSHEY_SIMPLEX;                              // choose a plain jane font
    double dblFontScale = (double)licPlate.imgPlate.rows / 30.0;            // base font scale on height of plate area
    int intFontThickness = (int)round_val(dblFontScale * 1.5,0);             // base font thickness on font scale
    int intBaseline = 0;
	
    cv::Size textSize = cv::getTextSize(licPlate.strChars, intFontFace, dblFontScale, intFontThickness, &intBaseline);      // call getTextSize

    ptCenterOfTextArea.x = (int)licPlate.rrLocationOfPlateInScene.center.x;         // the horizontal location of the text area is the same as the plate
	
    if (licPlate.rrLocationOfPlateInScene.center.y < (imgOriginalScene.rows * 0.75)) {      // if the license plate is in the upper 3/4 of the image
                                                                                            // write the chars in below the plate
        ptCenterOfTextArea.y = (int)round_val(licPlate.rrLocationOfPlateInScene.center.y,0) + (int)round_val((double)licPlate.imgPlate.rows * 1.6,0);
    } else {                                                                                // else if the license plate is in the lower 1/4 of the image
                                                                                            // write the chars in above the plate
        ptCenterOfTextArea.y = (int)round_val(licPlate.rrLocationOfPlateInScene.center.y,0) - (int)round_val((double)licPlate.imgPlate.rows * 1.6,0);
    }

    ptLowerLeftTextOrigin.x = (int)(ptCenterOfTextArea.x - (textSize.width / 2));           // calculate the lower left origin of the text area
    ptLowerLeftTextOrigin.y = (int)(ptCenterOfTextArea.y + (textSize.height / 2));          // based on the text area center, width, and height

             // write the text on the image
    cv::putText(imgOriginalScene, licPlate.strChars, ptLowerLeftTextOrigin, intFontFace, dblFontScale, SCALAR_YELLOW, intFontThickness);
}





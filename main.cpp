#include <stdio.h>
#include <iostream>
#include <stdio.h>
#include <iostream>
#include "opencv2/core.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/xfeatures2d.hpp"
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;
using namespace cv::xfeatures2d;
void readme();

class KeyPointClass{
	public:
		Point P;
		int index;
		double mean;
		vector<int> distances;

};

bool compareKeyPointsDist (int i,int j) {
	return (i < j);
}

bool compareKeyPointsMean (KeyPointClass i, KeyPointClass j) {
	return (i.mean < j.mean);
}


int main( int argc, char** argv ) // jako argv[1] podac zdjecie do znalezienia krokodyla, a na argv[2] sciezke folderu z wzorcami
{
  if( argc != 3 )
  { readme(); return -1; }
  Mat img_1 = imread( argv[1], IMREAD_GRAYSCALE );
  Mat img_2[16];
  img_2[0] = Mat::zeros(5,5,1);
  for (int i = 1; i <= 15; i++){
	  string path = argv[2] + string("/c") + to_string(i) + string(".jpg");
	  img_2[i] = imread( path, IMREAD_GRAYSCALE );
  }
  Mat MyGoodMatches = imread( argv[1], IMREAD_COLOR);
  if( !img_1.data || !img_2[15].data )
  { std::cout<< " --(!) Error reading images " << std::endl; return -1; }

  int minHessian = 400;
  Ptr<SURF> detector = SURF::create();
  detector->setHessianThreshold(minHessian);
  std::vector<KeyPoint> keypoints_1;
  std::vector<KeyPoint> keypoints_2[16];
  Mat descriptors_1;
  Mat descriptors_2[16];
  detector->detectAndCompute( img_1, Mat(), keypoints_1, descriptors_1 );
  for (int i = 1; i <= 15; i++){
	  detector->detectAndCompute( img_2[i], Mat(), keypoints_2[i], descriptors_2[i] );
  }

  FlannBasedMatcher matcher;
  std::vector< DMatch > matches[16];
  for (int i = 1; i <= 15; i++){
	  matcher.match( descriptors_1, descriptors_2[i], matches[i] );
  }

  std::vector< DMatch > good_matches;

  for (int j = 1; j <= 15; j++){
	  double max_dist = 0; double min_dist = 100;
	  for( int i = 0; i < descriptors_1.rows; i++ )
	  { double dist = matches[j][i].distance;
	    if( dist < min_dist ) min_dist = dist;
	    if( dist > max_dist ) max_dist = dist;
	  }

	  for( int i = 0; i < descriptors_1.rows; i++ )
	  { if( matches[j][i].distance <= max(2*min_dist, 0.02) )
		{ good_matches.push_back( matches[j][i]); }
	  }
  }

  vector<KeyPointClass> MyKeyPoints;
  for(vector<DMatch>::size_type i=0; i<good_matches.size(); i++)
  {
	KeyPointClass P;
  	P.P.x = keypoints_1[ good_matches[i].queryIdx].pt.x;
  	P.P.y = keypoints_1[ good_matches[i].queryIdx].pt.y;
  	MyKeyPoints.push_back(P);
  	//cout << "Point = " << P.P.x << ", " << P.P.y << endl;
  	//circle( MyGoodMatches, P.P, 5,  Scalar(0,0,255), 2, LINE_8, 0 );
  }

  for(unsigned int i = 0; i < MyKeyPoints.size(); i++) {
  	int x1 = MyKeyPoints[i].P.x;
  	int y1 = MyKeyPoints[i].P.y;
  	for(unsigned int j = 0; j < MyKeyPoints.size(); j++){
  		int dx = x1 - MyKeyPoints[j].P.x;
  		int dy = y1 - MyKeyPoints[j].P.y;
  		int distance = sqrt(dx*dx+dy*dy);
  		MyKeyPoints[i].distances.push_back(distance);
  	}
  	sort (MyKeyPoints[i].distances.begin(), MyKeyPoints[i].distances.end(), compareKeyPointsDist);
  	// policz srednią z 1/3 najblizszych dystansow
  	for(unsigned int k = 0; k < MyKeyPoints[i].distances.size()/3; k++){
  		MyKeyPoints[i].mean = MyKeyPoints[i].mean + MyKeyPoints[i].distances[k];
  	}
  	MyKeyPoints[i].mean = MyKeyPoints[i].mean/(MyKeyPoints[i].distances.size()/3);
  }

  // teraz sortowanie obliczonych punktow od najmniejszej do najwiekszej scredniej (mean)
  sort (MyKeyPoints.begin(), MyKeyPoints.end(), compareKeyPointsMean);

  for(unsigned int i = 0; i < MyKeyPoints.size()/3; i++){
	  cout << "Punkt : " << i << "  srednia odl. = " << MyKeyPoints[i].mean << endl;
	  circle( MyGoodMatches, MyKeyPoints[i].P, 5,  Scalar(0,0,255), 2, LINE_8, 0 );

  }

  for(unsigned int i = 0; i < MyKeyPoints.size()/9; i++){
	  circle( MyGoodMatches, MyKeyPoints[i].P, 5,  Scalar(255,0,0), 2, LINE_8, 0 );

  }

  namedWindow("My Good Matches", WINDOW_NORMAL);
  imshow( "My Good Matches", MyGoodMatches );


  //-- Show detected matches
  //namedWindow("Good Matches", WINDOW_NORMAL);
  //imshow( "Good Matches", img_matches );
  //for( int i = 0; i < (int)good_matches.size(); i++ ) { printf( "-- Good Match [%d] Keypoint 1: %d  -- Keypoint 2: %d  \n", i, good_matches[i].queryIdx, good_matches[i].trainIdx ); }
  waitKey(0);
  return 0;
}

/*
 * @function readme
 */
 
void readme()
{ std::cout << " Usage: ./surf_test_croc <img1> <img2>" << std::endl; }


/*  pseudo kod


wczytaj obrazy wzorcowe z folderu w src i wpisz do wektora ich deskryptory/keypointy/vectory keypointow

while (1){
	wez nowy/testowy obraz
	policz jego keypointy
	for wszystkie obrazy testowe {
		porownaj keypointy nowego obrazu z wzorcem (rysowanie kolorowych kresek)
		podziel keypointy na te co są w sąsiedstwie
	}


vector<cv::Point> MyKeyPoints;
for(vector<DMatch>::size_type i=0; i<good_matches.size(); i++)
{
	Point P;
	P.x = keypoints_1[ good_matches[i].queryIdx].pt.x;
	P.y = keypoints_1[ good_matches[i].queryIdx].pt.y;
	MyKeyPoints.pushback(P);
	cout << "Point = " << P.x << ", " << P.y << endl;
}

  std::vector< DMatch > good_matches;

keypoints_1[ matches[i].queryIdx].pt.x;
keypoints_1[ matches[i].queryIdx].pt.y;

keypoints_2[ matches[i].trainIdx].pt.x;
keypoints_2[ matches[i].trainIdx].pt.y;


}
vector<cv::Point> MyKeyPoints;

vector<vector<int> >

for(int i = 0; i < MyKeyPoints.size(); i++) {
	int x1 = MyKeyPoints[i].x;
	int y1 = MyKeyPoints[i].y;
	for(int j = 0; j < MyKeyPoints.size(); j++){
		int dx = x1 - MyKeyPoints[j].x;
		int dy = y1 - MyKeyPoints[j].y;
		int distance = sqrt(dx*dx+dy*dy);
	}
}

sort (localMaxVect.begin(), localMaxVect.end(), compareLocalMax);
*/




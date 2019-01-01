#include "Tile.h"
#include <iostream>
#include <math.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <stdio.h>
#include <vector>

const int FRAME_WIDTH  = 1920;
const int FRAME_HEIGHT = 1080;

// Binary conversion vars
int H_MIN = 0;
int H_MAX = 256;
int S_MIN = 0;
int S_MAX = 256;
int V_MIN = 0;
int V_MAX = 256;


//The size of the cubes displayed on the screen
int GRID_BOX_SIZE = 250;
int WIDTH_OFFSET = (FRAME_WIDTH/2)-(GRID_BOX_SIZE+(GRID_BOX_SIZE/2));
int HEIGHT_OFFSET = (FRAME_HEIGHT/2)-(GRID_BOX_SIZE+(GRID_BOX_SIZE/2));


// Weather we've started scanning the cube
bool started = false;

// The side we are currently scanning
int side = 0;

// The minimun required size of the contour
int CONTOUR_SIZE = GRID_BOX_SIZE*.5;

//The sizes for morphOps
int MORPH_ERODE_X = 16;
int MORPH_ERODE_Y = 16;

int MORPH_DILATE_X = 10;
int MORPH_DILATE_Y = 10;


// The different settings of the colors

//FIXME: This is off
const cv::Scalar RED_MIN = cv::Scalar(0, 136, 0);
const cv::Scalar RED_MAX = cv::Scalar(256, 240, 190);

const cv::Scalar BLUE_MIN = cv::Scalar(98, 166, 0);
const cv::Scalar BLUE_MAX = cv::Scalar(167, 256, 256);

const cv::Scalar YELLOW_MIN = cv::Scalar(19, 0, 0);
const cv::Scalar YELLOW_MAX = cv::Scalar(40, 256, 256);

const cv::Scalar ORANGE_MIN = cv::Scalar(1, 79, 0);
const cv::Scalar ORANGE_MAX = cv::Scalar(19, 256, 256);

//FIXME: Maybe have the filterObjects check and if it's not been found by any color, it must be white
const cv::Scalar WHITE_MIN = cv::Scalar(123, 0, 193);
const cv::Scalar WHITE_MAX = cv::Scalar(256,23, 256);

const cv::Scalar GREEN_MIN = cv::Scalar(37, 0, 0);
const cv::Scalar GREEN_MAX = cv::Scalar(80, 256, 256);

std::vector<Tile> tiles;

void on_trackbar(int, void *) {}

static void onMouse(int event, int x, int y, int, void *) {
  if (event == cv::EVENT_LBUTTONDOWN) started = true;
}

void setup(cv::Mat& mat){
	int stepSize = GRID_BOX_SIZE;

	int width = (stepSize*3);
	int height = (stepSize*3);

	for (int i = 0; i<height+stepSize; i += stepSize)
		cv::line(mat, cv::Point(0+WIDTH_OFFSET, i+HEIGHT_OFFSET), cv::Point(width+WIDTH_OFFSET, i+HEIGHT_OFFSET), cv::Scalar(0, 0, 0));

	for (int i = 0; i<width+stepSize; i += stepSize)
		cv::line(mat, cv::Point(i+WIDTH_OFFSET, 0+HEIGHT_OFFSET), cv::Point(i+WIDTH_OFFSET, height+HEIGHT_OFFSET), cv::Scalar(0, 0, 0));
}

void spawnTrackbars() {
	cv::namedWindow("Options", 0);

	cv::createTrackbar("Min object size",  "Options", &CONTOUR_SIZE, 500, on_trackbar);

	cv::createTrackbar("H_MIN", "Options", &H_MIN, H_MAX, on_trackbar);
	cv::createTrackbar("H_MAX", "Options", &H_MAX, H_MAX, on_trackbar);
	cv::createTrackbar("S_MIN", "Options", &S_MIN, S_MAX, on_trackbar);
	cv::createTrackbar("S_MAX", "Options", &S_MAX, S_MAX, on_trackbar);
	cv::createTrackbar("V_MIN", "Options", &V_MIN, V_MAX, on_trackbar);
	cv::createTrackbar("V_MAX", "Options", &V_MAX, V_MAX, on_trackbar);

	cv::createTrackbar("V_MIN", "Options", &V_MIN, V_MAX, on_trackbar);
	cv::createTrackbar("V_MAX", "Options", &V_MAX, V_MAX, on_trackbar);



	cv::createTrackbar("HEIGHT OFFSET", "Options", &HEIGHT_OFFSET, FRAME_WIDTH, on_trackbar);
	cv::createTrackbar("WIDTH OFFSET", "Options", &WIDTH_OFFSET, FRAME_HEIGHT, on_trackbar);
	cv::createTrackbar("GRID BOX SIZE", "Options", &GRID_BOX_SIZE, (FRAME_WIDTH/3), on_trackbar);

	cv::createTrackbar("MORPH ERODE X", "Options", &MORPH_ERODE_X, 50, on_trackbar);
	cv::createTrackbar("MORPH ERODE Y", "Options", &MORPH_ERODE_Y, 50, on_trackbar);

	cv::createTrackbar("MORPH DILATE X", "Options", &MORPH_DILATE_X, 50, on_trackbar);
	cv::createTrackbar("MORPH DILATE Y", "Options", &MORPH_DILATE_Y, 50, on_trackbar);
}

void morphOps(cv::Mat& thresh) {
	cv::Mat erodeElement  = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(MORPH_ERODE_X, MORPH_ERODE_Y));
	cv::Mat dilateElement = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(MORPH_DILATE_X, MORPH_DILATE_Y));

	cv::erode(thresh, thresh, erodeElement);
	cv::erode(thresh, thresh, erodeElement);

	cv::dilate(thresh, thresh, dilateElement);
	cv::dilate(thresh, thresh, dilateElement);
}

void filterObjects(cv::Mat &binaryFeed, cv::Mat &rawFeed, std::string color, int side) {
	cv::Mat temp;
	binaryFeed.copyTo(temp);

	// these two vectors needed for output of findContours
	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;

	cv::findContours(temp, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

	if (hierarchy.size() > 0) {
		for (int index = 0; index >= 0; index = hierarchy[index][0]) {
			cv::Moments moment = moments((cv::Mat)contours[index]);
			double area        = moment.m00;


			double long x = moment.m10 / area;
			double long y = moment.m01 / area;

				// If it's less than 50x50, ignore it
			if ((area > (CONTOUR_SIZE * CONTOUR_SIZE))){
				if( (x > WIDTH_OFFSET && x < (WIDTH_OFFSET+(GRID_BOX_SIZE*3)) ) && (y > HEIGHT_OFFSET && y < (HEIGHT_OFFSET+(GRID_BOX_SIZE*3)))) {
					//TODO: Make this a constructor
					Tile tile;
					tile.x   = x;
					tile.y   = y;
					tile.positionX = ( (tile.x-WIDTH_OFFSET) / GRID_BOX_SIZE )+1;
					tile.positionY = ( (tile.y-HEIGHT_OFFSET) / GRID_BOX_SIZE )+1;
					tile.area  = area;
					tile.side  = side;
					tile.color = color;

					bool found;
					Tile foundTile;
					for(unsigned int i = 0; i < tiles.size(); i++){
						if(tiles[i].positionX == tile.positionX && tiles[i].positionY == tile.positionY && !tiles[i].color.empty() && tiles[i].color != tile.color){
							found = true;
							foundTile = tiles[i];
						}
					}

					if(!found){
						cv::Scalar fillColor;

						//FIXME: This should be a switch
						if (tile.color == "red") fillColor = cv::Scalar(0, 0, 255);

						if (tile.color == "blue") fillColor = cv::Scalar(255, 0, 0);

						if (tile.color == "yellow") fillColor = cv::Scalar(102, 255, 255);

						if (tile.color == "white") fillColor = cv::Scalar(255, 255, 255);

						if (tile.color == "green") fillColor = cv::Scalar(0, 255, 0);

						if (tile.color == "orange") fillColor = cv::Scalar(0, 165, 255);

						tile.fillColor = fillColor;

						tiles.push_back(tile);
					} else {
						printf("WARNING: Found cube at position: (%d,%d) with color %s, but already cube with color %s there \n", foundTile.positionX, foundTile.positionY, foundTile.color.c_str(), tile.color.c_str());
					}
				}
			}
		}
	}

	for(unsigned int i = 0; i < tiles.size(); i++){
		cv::rectangle(
			rawFeed,
			cv::Point( (WIDTH_OFFSET+((tiles[i].positionX-1)*GRID_BOX_SIZE)), (HEIGHT_OFFSET+((tiles[i].positionY-1)*GRID_BOX_SIZE))),
			cv::Point( (WIDTH_OFFSET+((tiles[i].positionX)*GRID_BOX_SIZE)), (HEIGHT_OFFSET+((tiles[i].positionY)*GRID_BOX_SIZE))),
			cv::Scalar(tiles[i].fillColor)
		);

		char output[50];
		sprintf(output, "(%d,%d) %s", tiles[i].positionX, tiles[i].positionY, tiles[i].color.c_str() );

		cv::putText(rawFeed,output, cv::Point(tiles[i].x, tiles[i].y), 1, 1, tiles[i].fillColor);
	}
}

void drawResults(std::vector<std::vector<Tile> >cube) {
	cv::Mat image = cv::Mat::zeros(1920, 1080, CV_8UC3);

	for (unsigned int i = 0; i < cube.size(); i++) {
		for (unsigned int j = i; j < cube[i].size(); j++) {
			Tile tile = cube[i][j];
			cv::rectangle(
				image, cv::Point( (((tile.positionX * 250) - 250)*i), ((tile.positionY * 250) - 250)*i), cv::Point(((tile.positionX * 250) * i), ((tile.positionY * 250) * i)), tile.fillColor, -1);
		}
	}

	imshow("Results", image);
}

int main(int argc, char *argv[]) {
  spawnTrackbars();

  // raw camera feed
  cv::Mat RawFeed;

  // HSV feed
  cv::Mat HSVFeed;

  // Binary feed
  cv::Mat BinaryFeed;

  // The results feed
  cv::Mat ResultsFeed;

  // The debug feed
  cv::Mat DebugFeed;

  // The list of all the tiles on each of the sides
  std::vector<std::vector<Tile> > cube;

  // Get main video capture
  cv::VideoCapture capture;

  // Open
  capture.open(0);

  // Set to correct size
  capture.set(CV_CAP_PROP_FRAME_WIDTH,  FRAME_WIDTH);
  capture.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);

  cv::waitKey(2000);

  // The color we are currently scanning
  std::string color = "";

  while (1) {
	//Setup the grid

	cv::setMouseCallback("Camera", onMouse);
	capture.read(RawFeed);

	if (started) {
			started = false;


			// We are done scanning the side..
			printf("%s\n", "Done with side");

			cube.push_back(tiles);
			side++;
	}

	if(!tiles.empty()){
			tiles.clear();
	}

	setup(RawFeed);

	//Convert feed to HSV
	cvtColor(RawFeed, HSVFeed, cv::COLOR_BGR2HSV);


	color = "red";
	inRange(HSVFeed, RED_MIN, RED_MAX, BinaryFeed);
	morphOps(BinaryFeed);
	filterObjects(BinaryFeed, RawFeed, color, side);


	color = "yellow";
	inRange(HSVFeed, YELLOW_MIN, YELLOW_MAX, BinaryFeed);
	morphOps(BinaryFeed);
	filterObjects(BinaryFeed, RawFeed, color, side);

	color = "orange";
	inRange(HSVFeed, ORANGE_MIN, ORANGE_MAX, BinaryFeed);
	morphOps(BinaryFeed);
	filterObjects(BinaryFeed, RawFeed, color, side);

	color = "white";
	inRange(HSVFeed, WHITE_MIN, WHITE_MAX, BinaryFeed);
	morphOps(BinaryFeed);
	filterObjects(BinaryFeed, RawFeed, color, side);

	color = "green";
	inRange(HSVFeed, GREEN_MIN, GREEN_MAX, BinaryFeed);
	morphOps(BinaryFeed);
	filterObjects(BinaryFeed, RawFeed, color, side);


	color = "blue";
	inRange(HSVFeed, BLUE_MIN, BLUE_MAX, BinaryFeed);
	morphOps(BinaryFeed);
	filterObjects(BinaryFeed, RawFeed, color, side);

	inRange(HSVFeed, cv::Scalar(H_MIN, S_MIN, V_MIN), cv::Scalar(H_MAX, S_MAX, V_MAX), DebugFeed);
	morphOps(DebugFeed);
	imshow("Debug",  DebugFeed);


	imshow("Camera", RawFeed);
	cv::waitKey(1);
  }

  return 0;
}

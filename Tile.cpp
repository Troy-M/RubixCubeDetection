#include "Tile.h"
#include "string"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/videoio.hpp>

Tile::Tile() {}

Tile::~Tile(void) {}


Tile::Tile(int         positionX,
           int         positionY,
           int         side,
           int        x,
           int        y,
           int        area,
           cv::Scalar  fillColor,
           std::string color) {

  Tile::positionX = positionX;
  Tile::positionY = positionY;
  Tile::color     = color;
  Tile::side      = side;
  Tile::x         = x;
  Tile::y         = y;
  Tile::area      = area;
  Tile::fillColor = fillColor;
}

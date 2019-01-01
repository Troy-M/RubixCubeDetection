#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/videoio.hpp>

class Tile {
public:

  Tile();
  ~Tile(void);
  Tile(int        positionX,
       int        positionY,
       int        side,
       int       x,
       int       y,
       int       area,
       cv::Scalar  fillColor,
       std::string color);

  int  positionY, positionX, side;
  long x, y, area;
  std::string color;
  cv::Scalar  fillColor;

private:
};

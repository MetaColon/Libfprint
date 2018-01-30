#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>

#include "Algorithm.hpp"

using namespace cv;

// TODO Common Region extraction
// TODO => Rotation, Shifting, Scaling (=> library?)
// TODO Cross Correlation
// TODO => IFT?

/// Global variables

Mat src, src_gray;
Mat blurred, detected_edges;
Mat laplaced;

int edgeThresh = 1;
int lowThreshold;
int const max_lowThreshold = 100;
int ratio = 3;
int kernel_size = 3;

/**
 * @function ThinCanny
 */
void ThinCanny()
{
    /// Reduce noise with a kernel 2x2
    //TODO evaluate perfect kernel size
    blur(src_gray, detected_edges, Size(2, 2));

    /// Canny detector
    Canny(detected_edges, detected_edges, lowThreshold, lowThreshold * ratio, kernel_size);

    /// Using Canny's output as a mask, we calculate our result
    blurred = Scalar::all(0);

    src.copyTo(blurred, detected_edges);
}

int alterImage (char *fileName)
{
    /// Load an image
    src = imread(fileName);

    if (!src.data)
    { return -1; }

    /// Create a matrix of the same type and size as src (for dst)
    blurred.create(src.size(), src.type());

    /// Convert the image to grayscale
    cvtColor(src, src_gray, CV_BGR2GRAY);

    /// Calculate the thinned image
    ThinCanny();

    /// Generate the laplacian gradient image
    Laplacian(blurred, laplaced, CV_64F);

    return 0;
}

/** @function main */
int main(int argc, char **argv)
{
    alterImage(argv[1]);
    imwrite(argv[2], laplaced);
    return 0;
}
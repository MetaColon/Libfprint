#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>

#include "Algorithm.hpp"

using namespace cv;
using namespace std;

// TODO Common Region extraction
// TODO => Rotation, Shifting, Scaling (=> library?)
// TODO Cross Correlation
// TODO => IFT?

/// Global variables

Mat src, src_gray;
Mat blurred, blur_current;
Mat gradientX, gradientY;

// TODO optimise values
int lowThreshold = 30;
int ratio = 3;
int kernel_size = 3;
int blockSizeX = 16, blockSizeY = 8;
int countX, countY;

/**
 * @function ThinCanny
 */
void ThinCanny()
{
    /// Reduce noise with a kernel 3x3
    // TODO evaluate perfect kernel size
    blur(src_gray, blur_current, Size(3, 3));

    /// Canny detector
    Canny(blur_current, blur_current, lowThreshold, lowThreshold * ratio, kernel_size);

    /// Using Canny's output as a mask, we calculate our result
    blurred = Scalar::all(0);

    src.copyTo(blurred, blur_current);
}

/// Creates the field orientation image
double** getFieldOrientationImage (char *fileName)
{
    /// Load an image
    src = imread(fileName);

    if (!src.data)
    { return nullptr; }

    /// Create a matrix of the same type and size as src (for dst)
    blurred.create(src.size(), src.type());

    /// Convert the image to grayscale
    cvtColor(src, src_gray, CV_BGR2GRAY);

    /// Calculate the thinned image
    ThinCanny();

    /// Generate the gradient image in X and Y direction
    Sobel(blurred, gradientX, CV_64F, 0, 1);
    Sobel(blurred, gradientY, CV_64F, 1, 0);

    /// The size of the blocks
    // TODO differently sized blocks
    countX = gradientX.cols / blockSizeX;
    countY = gradientY.rows / blockSizeY;

    /// Get the actual blocks
    Mat** gradientXBlocks = getBlocks(blockSizeX, blockSizeY, countX, countY, gradientX);
    Mat** gradientYBlocks = getBlocks(blockSizeX, blockSizeY, countX, countY, gradientY);

    /// Get the orientations for the blocks
    double** orientations = nullptr;
    orientations = new double*[countY];

    for (int y = 0; y < countY; ++y)
    {
        orientations[y] = new double[countX];
        for (int x = 0; x < countX; ++x)
        {
            orientations[y][x] = getOrientationOfBlock(gradientXBlocks[y][x], gradientYBlocks[y][x], countX, countY);
        }
    }

    return orientations;
}

/// Get the blocks out of the whole image
Mat** getBlocks (int blockSizeX, int blockSizeY, int countX, int countY, const Mat &image)
{
    Mat** fin = nullptr;
    fin = new Mat*[countY];

    for (int y = 0; y < countY; ++y)
    {
        fin[y] = new Mat[countX];
        for (int x = 0; x < countX; ++x)
        {
            fin[y][x] = image.colRange(x, x + blockSizeX).rowRange(y, y + blockSizeY);
        }
    }

    return fin;
}

/// Evaluate the field orientation for the block
double getOrientationOfBlock (const Mat &blockX, const Mat &blockY, int sizeX, int sizeY)
{
    // I didn't invent this formula and hence can't explain it
    double a = 0, b = 0;
    for (int i = 0; i < sizeX; ++i)
    {
        for (int j = 0; j < sizeY; ++j)
        {
            a += 2 * blockX.at<double>(i, j) * blockY.at<double>(i, j);
            b += pow(blockX.at<double>(i, j), 2) - pow(blockY.at<double>(i, j), 2);
        }
    }

    double result = 0.5 * atan(a / b);

    return result;
}

/// Cross correlate the field orientation images
double crossCorrelation(double **verify, double **enrolled, int sizeY, int sizeX)
{
    /// The count of the pixels
    int count = sizeX * sizeY;

    /// The sum of the common values
    double sum = 0;
    for (int y = 0; y < sizeY; ++y)
    {
        for (int x = 0; x < sizeX; ++x)
        {
            /// We don't want to take a look at the nan (black) parts of the field orientation image
            if (isnan(verify[y][x]) || isnan(enrolled[y][x]))
            {
                count--;
                continue;
            }
            /// Get the common value of the orientations (varies from 0 to 1)
            sum += commonValueOfOrientation(verify[y][x], enrolled[y][x]);
        }
    }

    return sum / count;
}

double commonValueOfOrientation (double a, double b)
{
    /// The orientation in radians
    double radianA = a * 2 * 3.14159265358979323;
    double radianB = b * 2 * 3.14159265358979323;
    /// The x part of the direction
    double ax = cos(radianA);
    /// The y part of the direction
    double ay = sin(radianA);
    double bx = cos(radianB);
    double by = sin(radianB);

    /// Multiply the vectors using the dot product
    double result = ax * bx + ay * by;

    return result;
}

/** @function main */
int main(int argc, char **argv)
{
    /// Get the image to verify - insert the location of the images as strings for now
    // TODO get path as parameters
    double** verify = getFieldOrientationImage("C:\\Users\\Meta Colon\\Desktop\\fp\\verify.png");
    double** enrolled = getFieldOrientationImage("C:\\Users\\Meta Colon\\Desktop\\fp\\fingerprint.png");
    double result = crossCorrelation(verify, enrolled, countY, countX);
    cout << "Result: " << result << "\n";
    return 0;
}
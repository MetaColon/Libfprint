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

int edgeThresh = 1;
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
    //TODO evaluate perfect kernel size
    blur(src_gray, blur_current, Size(3, 3));

    imwrite("C:\\Users\\Meta Colon\\Desktop\\fp\\blur.png", blur_current);

    /// Canny detector
    Canny(blur_current, blur_current, lowThreshold, lowThreshold * ratio, kernel_size);

    /// Using Canny's output as a mask, we calculate our result
    blurred = Scalar::all(0);

    src.copyTo(blurred, blur_current);

    imwrite("C:\\Users\\Meta Colon\\Desktop\\fp\\canny.png", blurred);
}

double** alterImage (char *fileName)
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

    imwrite("C:\\Users\\Meta Colon\\Desktop\\fp\\gradientX.png", gradientX);
    imwrite("C:\\Users\\Meta Colon\\Desktop\\fp\\gradientY.png", gradientY);

    countX = gradientX.cols / blockSizeX;
    countY = gradientY.rows / blockSizeY;

    cout << "countX: " << countX << "\ncountY: " << countY << "\n";

    Mat** gradientXBlocks = getBlocks(blockSizeX, blockSizeY, countX, countY, gradientX);
    Mat** gradientYBlocks = getBlocks(blockSizeX, blockSizeY, countX, countY, gradientY);

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

double getOrientationOfBlock (const Mat &blockX, const Mat &blockY, int sizeX, int sizeY)
{
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

double crossCorrelation(double **verify, double **enrolled, int sizeY, int sizeX)
{
    int count = sizeX * sizeY;
    double sum = 0;
    for (int y = 0; y < sizeY; ++y)
    {
        for (int x = 0; x < sizeX; ++x)
        {
            cout << "Orientation at " << x << "/" << y << ": " << verify[y][x] << "/" << enrolled[y][x] << "\n";

            if (isnan(verify[y][x]) || isnan(enrolled[y][x]))
            {
                count--;
                continue;
            }
            sum += commonValueOfOrientation(verify[y][x], enrolled[y][x]);
        }
    }

    return sum / count;
}

double commonValueOfOrientation (double a, double b)
{
    double radianA = a * 2 * 3.14159265358979323;
    double radianB = b * 2 * 3.14159265358979323;
    double ax = cos(radianA);
    double ay = sin(radianA);
    double bx = cos(radianB);
    double by = sin(radianB);

    double result = ax * bx + ay * by;

    return result;
}

/** @function main */
int main(int argc, char **argv)
{
    double** verify = alterImage("C:\\Users\\Meta Colon\\Desktop\\fp\\verify.png");
    cout << "\n";
    double** enrolled = alterImage("C:\\Users\\Meta Colon\\Desktop\\fp\\fingerprint.png");
    double result = crossCorrelation(verify, enrolled, countY, countX);
    cout << "Result: " << result << "\n";
    return 0;
}
//
// Created by Meta Colon on 30/01/2018.
//

#ifndef ALGORITHM_ALGORITHM_H
#define ALGORITHM_ALGORITHM_H

#include <zconf.h>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <cstdlib>
#include <cstdio>

#include "Algorithm.h"

using namespace cv;

struct twoDarray;

void ThinCanny();

twoDarray getFieldOrientationImage(char *fileName);

twoDarray getFieldOrientationImage(Mat img);

Mat **getBlocks(int blockSizeX, int blockSizeY, int countX, int countY, const Mat &image);

double getOrientationOfBlock(const Mat &blockX, const Mat &blockY, int sizeX, int sizeY);

double crossCorrelation(double **verify, double **enrolled, int sizeX, int sizeY);

double commonValueOfOrientation(double a, double b);

double getMatchPercentage(char *path1, char *path2);

double getMatchPercentage(Mat img1, Mat img2);

int main(int argc, char **argv);

#endif //ALGORITHM_ALGORITHM_H

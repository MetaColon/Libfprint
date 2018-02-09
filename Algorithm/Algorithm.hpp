//
// Created by Meta Colon on 30/01/2018.
//

#ifndef ALGORITHM_ALGORITHM_H
#define ALGORITHM_ALGORITHM_H

#include <zconf.h>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>

#include "Algorithm.hpp"

using namespace cv;

void ThinCanny();

double **alterImage(char *fileName);

Mat **getBlocks(int blockSizeX, int blockSizeY, int countX, int countY, const Mat &image);

double getOrientationOfBlock(const Mat &blockX, const Mat &blockY, int sizeX, int sizeY);

double crossCorrelation(double **verify, double **enrolled, int sizeY, int sizeX);

double commonValueOfOrientation(double a, double b);

#endif //ALGORITHM_ALGORITHM_H

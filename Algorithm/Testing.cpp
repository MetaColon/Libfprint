//
// Created by Meta Colon on 11/02/2018.
//

#include <cstdio>
#include <cstdlib>
#include <libusb-1.0>
#include <fp_internal.h>
#include <assembling.h>
#include <libfprint>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "Algorithm.h"
#include "driver/elan.h"

#include "Testing.h"

using namespace cv;

struct fp_dscv_dev *discover_device(struct fp_dscv_dev **discovered_devs)
{
    struct fp_dscv_dev *ddev = discovered_devs[0];
    struct fp_driver *drv;
    if (!ddev)
        return NULL;

    drv = fp_dscv_dev_get_driver(ddev);
    printf("Found device claimed by %s driver\n", fp_driver_get_full_name(drv));
    return ddev;
}

int main(int argc, char **argv)
{
    int r = 1;
    /*struct fp_dscv_dev *ddev;
    struct fp_dscv_dev **discovered_devs;
    struct fp_dev *dev;
    struct fp_img *img = NULL;

    r = fp_init();
    if (r < 0)
    {
        fprintf(stderr, "Failed to initialize libfprint\n");
        exit(1);
    }
    fp_set_debug(3);

    discovered_devs = fp_discover_devs();
    if (!discovered_devs)
    {
        fprintf(stderr, "Could not discover devices\n");
        goto out;
    }

    ddev = discover_device(discovered_devs);
    if (!ddev)
    {
        fprintf(stderr, "No devices detected.\n");
        goto out;
    }

    dev = fp_dev_open(ddev);
    fp_dscv_devs_free(discovered_devs);
    if (!dev)
    {
        fprintf(stderr, "Could not open device.\n");
        goto out;
    }

    if (!fp_dev_supports_imaging(dev))
    {
        fprintf(stderr, "this device does not have imaging capabilities.\n");
        goto out_close;
    }

    printf("Opened device. It's now time to scan your finger.\n\n");

    r = fp_dev_img_capture(dev, 0, &img);
    if (r)
    {
        fprintf(stderr, "image capture failed, code %d\n", r);
        goto out_close;
    }

    r = fp_img_save_to_file(img, "finger.pgm");
    if (r)
    {
        fprintf(stderr, "img save failed, code %d\n", r);
        goto out_close;
    }*/

    FILE *file = fopen("C:\\Users\\Meta Colon\\Desktop\\fp\\enrolled.pgm", "r");

    fseek(file, 0, SEEK_END);
    long lSize = ftell(file);
    rewind(file);

    char * buffer = (char*) malloc(sizeof(char) *lSize);

    struct fp_img *img = reinterpret_cast<fp_img *>(fread(buffer, 1, lSize, file));

    Mat mat = Mat(img->width, img->height, CV_32F, img);

    r = 0;
    out_close:
    //fp_dev_close(dev);
    out:
    fp_exit();
    return r;
}
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <libusb-1.0/libusb.h>

#define BULK_EP1_OUT    0x01
#define BULK_EP2_IN     0x82
#define BULK_EP3_IN     0x83

#define IMAGE_PACKET_SIZE (96 * 96 * 2)
#define SIZE_STORE 256

unsigned char temp_store[SIZE_STORE];
int width, height, img_buf_len;
char *img_buf;

char set_cmd_reset[2] = {0x40, 0x11};
char get_cmd_status[2] = {0x40, 0x13};
char set_cmd_fuse_load[2] = {0x40, 0x14};
char get_cmd_image_size[2] = {0x00, 0x0C};
char get_cmd_version[2] = {0x40, 0x19};
char get_cmd_calibration[2] = {0x40, 0x23};
char get_cmd_calibration_mean[2] = {0x40, 0x24};
char get_cmd_waitforfinger[2] = {0x40, 0x3f};
char get_cmd_image[2] = {0x00, 0x09};


char set_cmd_stop_prescan[2] = {0x00, 0x0b};
char get_cmd_cal_mean[2] = {0x40, 0x24};


int write_to_file(char *buf, int size) {
    FILE *fp;
    fp = fopen("./out.fp", "w");
    if (fp < 0) return 1;
    fwrite(buf, 1, size, fp);
    fclose(fp);
    return 0;
}

int main(int argc, char *argv[]) {
    int r0, i, devs_count = 0, libusb_config = 0, transferred = 0;
    int id = 0;

    struct libusb_device **devs;
    struct libusb_device *dev;
    struct libusb_device_descriptor desc;
    struct libusb_device_handle *handle = NULL;

    if (argc == 2) {
        id = atoi((const char *) argv[1]);
    }

    r0 = libusb_init(NULL);
    if (r0 < 0) return 1;

    devs_count = libusb_get_device_list(NULL, &devs);
    if (devs_count < 0) {
        libusb_exit(NULL);
        printf("Error %d\n", r0);
        return 2;
    }

    while (1) {
        dev = devs[i];
        if (dev != NULL) {
            r0 = libusb_get_device_descriptor(dev, &desc);
            if (r0 < 0) continue;

            if ((desc.idVendor == 0x04f3) && (desc.idProduct == 0x0903) ||
                (desc.idVendor == 0x04f3) && (desc.idProduct == 0x0907) ||
                (desc.idVendor == 0x04f3) && (desc.idProduct == 0x0c03) ||
                (desc.idVendor == 0x04f3) && (desc.idProduct == 0x0c16) ||
                (desc.idVendor == 0x04f3) && (desc.idProduct == 0x0c26) ||
                (desc.idVendor == 0x04f3) && (desc.idProduct == 0x0c1a)) {
                r0 = 0;
                printf("Device with vid %x pid %x found.\n", desc.idVendor, desc.idProduct);
                break;
            }
        } else {
            r0 = 3;
            break;
        }

        i++;
        if (i > devs_count) {
            r0 = 99;
            goto app_exit;
        }
    }

    if (r0 != 0)
        goto app_exit;

    /*****************************************************/

    r0 = libusb_open(dev, &handle);
    if (r0 < 0) {
        r0 = 5;
        goto app_exit;
    }

    r0 = libusb_get_configuration(handle, &libusb_config);
    if (r0 != 0) {
        r0 = 98;
        goto app_exit_2;
    }

    printf("Config number is %d\n", libusb_config);

    if (libusb_config > 1) {
        r0 = libusb_set_configuration(handle, 1);
        if (r0 != 0) {
            r0 = 97;
            goto app_exit_2;
        }
        printf("Config set to 1\n");
    }

    if (libusb_kernel_driver_active(handle, 0) == 1) {
        if (libusb_detach_kernel_driver(handle, 0) != 0) {
            r0 = 96;
            goto app_exit_2;
        }
    }

    r0 = libusb_claim_interface(handle, 0);
    if (r0 != 0) {
        r0 = 95;
        goto app_exit_2;
    }

    /* reset */

    r0 = libusb_bulk_transfer(handle, BULK_EP1_OUT, set_cmd_reset, 2, &transferred, 0);
    if ((r0 == 0) && (transferred == 2)) {
        printf("CMD RESET sent\n");
    }
    usleep(5000);
    r0 = libusb_bulk_transfer(handle, BULK_EP1_OUT, set_cmd_fuse_load, 2, &transferred, 0);
    if ((r0 == 0) && (transferred == 2)) {
        printf("CMD Fuse Load sent\n");
    }

    /* version */

    r0 = libusb_bulk_transfer(handle, BULK_EP1_OUT, get_cmd_version, 2, &transferred, 0);
    if ((r0 == 0) && (transferred == 2)) {
        printf("CMD VERSION sent\n");
    }
    r0 = libusb_bulk_transfer(handle, BULK_EP3_IN, temp_store, 2, &transferred, 0);
    printf("FP Bridge FW Version %d.%d\n", temp_store[0], temp_store[1]);

    /* image size */

    r0 = libusb_bulk_transfer(handle, BULK_EP1_OUT, get_cmd_image_size, 2, &transferred, 0);
    if ((r0 == 0) && (transferred == 2)) {
        printf("CMD Get Image Size sent\n");
    }
    r0 = libusb_bulk_transfer(handle, BULK_EP3_IN, temp_store, 4, &transferred, 0);
    width = temp_store[0];
    height = temp_store[2];
    printf("Width x Height = %dx%d\n", width, height);
    img_buf_len = 2 * width * height;
    img_buf = malloc(img_buf_len);

    /* calibration */

    do {
        /* calibration image mean value */
        r0 = libusb_bulk_transfer(handle, BULK_EP1_OUT, get_cmd_calibration_mean, 2, &transferred,
                                  0);
        if ((r0 == 0) && (transferred == 2)) {
            printf("CMD Get Calibration Mean sent\n");
        }
        r0 = libusb_bulk_transfer(handle, BULK_EP3_IN, temp_store, 2, &transferred, 0);
        i = ((unsigned char) temp_store[0] << 8 | (unsigned char) temp_store[1]) & 0x0FFFF;
        printf("calibration mean value: %d (0x%x)\n", i, i);

        if (i > 500) {
            unsigned char result = 0;

            /* calibration */
            r0 = libusb_bulk_transfer(handle, BULK_EP1_OUT, get_cmd_calibration, 2, &transferred,
                                      0);
            if ((r0 == 0) && (transferred == 2)) {
                printf("CMD CALIBRATION sent\n");
            }
            r0 = libusb_bulk_transfer(handle, BULK_EP3_IN, &result, 1, &transferred, 0);
            printf("Calibration Status: 0x%x\n", result);

            if (result == 0x03) break; /* FW>1.53? */

            /* Seems to update calibration image mean value. FW<=1.53? */
            r0 = libusb_bulk_transfer(handle, BULK_EP1_OUT, get_cmd_status, 2, &transferred, 0);
            if ((r0 == 0) && (transferred == 2)) {
                printf("CMD STATUS sent\n");
            }
            r0 = libusb_bulk_transfer(handle, BULK_EP3_IN, &result, 1, &transferred, 0);
            printf("Status: 0x%x\n", result);

            usleep(5000);
        }
    } while (i > 500);

    /* wait for image */

    r0 = libusb_bulk_transfer(handle, BULK_EP1_OUT, get_cmd_waitforfinger, 2, &transferred, 0);
    if ((r0 == 0) && (transferred == 2)) {
        printf("CMD Wait For Finger sent\n");
    }
    r0 = libusb_bulk_transfer(handle, BULK_EP3_IN, temp_store, 1, &transferred, 0);
    printf("Received 0x%x\n", temp_store[0]);

    /* get image */

    r0 = libusb_bulk_transfer(handle, BULK_EP1_OUT, get_cmd_image, 2, &transferred, 0);
    if ((r0 == 0) && (transferred == 2)) {
        printf("CMD Get Image sent\n");
    }
    r0 = libusb_bulk_transfer(handle, BULK_EP2_IN, img_buf, img_buf_len, &transferred, 0);
    printf("Received %d\n", transferred);

    write_to_file(img_buf, img_buf_len);

    snprintf(temp_store, SIZE_STORE, "convert -depth 16 -size %dx%d+0 gray:out.fp ./out.png", width,
             height);
    system(temp_store);
    system("rm ./out.fp");

    app_exit_2:
    libusb_close(handle);

    /*****************************************************/

    app_exit:
    libusb_free_device_list(devs, 1);
    libusb_exit(NULL);

    if (r0 != 0) {
        printf("Error %d\n", r0);
    }

    return r0;
}
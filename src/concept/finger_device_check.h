#ifndef FINGER_DEVICE_CHECK_H
#define FINGER_DEVICE_CHECK_H


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "myfprint.h"
//int minutiaCount=0;
struct fp_dscv_dev *status_discover_device(struct fp_dscv_dev **discovered_devs)
{
    struct fp_dscv_dev *ddev = discovered_devs[0];
    struct fp_driver *drv;
    if (!ddev)
        return NULL;

    drv = fp_dscv_dev_get_driver(ddev);
    //printf("Found device claimed by %s driver\n", fp_driver_get_full_name(drv));
    return ddev;
}

static bool fingerDeviceExists(void)
{

    int r = 1;
    struct fp_dscv_dev *ddev;
    struct fp_dscv_dev **discovered_devs;
    struct fp_dev *dev;

    r = fp_init();
    if (r < 0) {
        //fprintf(stderr, "Failed to initialize libfprint\n");
        goto failed_out;
    }

    discovered_devs = fp_discover_devs();
    if (!discovered_devs) {
        //fprintf(stderr, "Could not discover devices\n");
        goto failed_out;

    }

    ddev = status_discover_device(discovered_devs);
    if (!ddev) {
        //fprintf(stderr, "No devices detected.\n");
        goto failed_out;

    }

    dev = fp_dev_open(ddev);
    fp_dscv_devs_free(discovered_devs);
    if (!dev) {
        //fprintf(stderr, "Could not open device.\n");
        goto failed_out;
    }
    fp_dev_close(dev);
    return true;

    failed_out:
    fp_exit();
    return false;
}


#endif // FINGER_DEVICE_CHECK_H

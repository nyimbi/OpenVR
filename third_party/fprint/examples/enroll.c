/*
 * Example fingerprint enrollment program
 * Enrolls your right index finger and saves the print to disk
 * Copyright (C) 2007 Daniel Drake <dsd@gentoo.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <libfprint/fprint.h>
int minutiaCount=0;
struct fp_dscv_dev *discover_device(struct fp_dscv_dev **discovered_devs)
{
	struct fp_dscv_dev *ddev = discovered_devs[0];
	struct fp_driver *drv;
	if (!ddev)
		return NULL;
	
	drv = fp_dscv_dev_get_driver(ddev);
//	printf("Found device claimed by %s driver\n", fp_driver_get_full_name(drv));
	return ddev;
}

struct fp_print_data *enroll(struct fp_dev *dev) {
	struct fp_print_data *enrolled_print = NULL;
	int r;

//	printf("You will need to successfully scan your finger %d times to complete the process.\n", fp_dev_get_nr_enroll_stages(dev));

//	do {
		struct fp_img *img = NULL;
	
//		sleep(1);
//		printf("\nScan your finger now.\n");

		r = fp_enroll_finger_img(dev, &enrolled_print, &img);
		if (img) {
                        fp_img_standardize(img);
			

	                struct fp_minutia** minutiaList = fp_img_get_minutiae(img,&minutiaCount);
                        fp_img_save_to_file(img, "enrolled_normal.pgm");

			struct fp_img *binaryImage = fp_img_binarize(img);
			const char *enrollFile;
			enrollFile = "enrolled_binarized.pgm";
		        fp_img_save_to_file(binaryImage, "enrolled_binarized.pgm");

			fp_img_free(img);
		}
		if (r < 0) {
			printf("%d",r);
			return NULL;
		}
/*
		switch (r) {
		case FP_ENROLL_COMPLETE:
			printf("Enroll complete!\n");
			break;
		case FP_ENROLL_FAIL:
			printf("Enroll failed, something wen't wrong :(\n");
			return NULL;
		case FP_ENROLL_PASS:
			printf("Enroll stage passed. Yay!\n");
			break;
		case FP_ENROLL_RETRY:
			printf("Didn't quite catch that. Please try again.\n");
			break;
		case FP_ENROLL_RETRY_TOO_SHORT:
			printf("Your swipe was too short, please try again.\n");
			break;
		case FP_ENROLL_RETRY_CENTER_FINGER:
			printf("Didn't catch that, please center your finger on the "
				"sensor and try again.\n");
			break;
		case FP_ENROLL_RETRY_REMOVE_FINGER:
			printf("Scan failed, please remove your finger and then try "
				"again.\n");
			break;
		}
	} while (r != FP_ENROLL_COMPLETE);
*/
	if (!enrolled_print) {
		printf("%d",-103);
		return NULL;
	}

//	printf("Enrollment completed!\n\n");
	return enrolled_print;
}

int main(void)
{
	int r = 1;
	struct fp_dscv_dev *ddev;
	struct fp_dscv_dev **discovered_devs;
	struct fp_dev *dev;
	struct fp_print_data *data;
/*
	printf("This program will enroll your right index finger, "
		"unconditionally overwriting any right-index print that was enrolled "
		"previously. If you want to continue, press enter, otherwise hit "
		"Ctrl+C\n");
	getchar();
*/
	r = fp_init();
	if (r < 0) {
//		fprintf(stderr, "Failed to initialize libfprint\n");
		printf("%d",-101);
		exit(1);
	}

	discovered_devs = fp_discover_devs();
	if (!discovered_devs) {
//		fprintf(stderr, "Could not discover devices\n");
		printf("%d",-100);
		goto out;
	}

	ddev = discover_device(discovered_devs);
	if (!ddev) {
		printf("%d",-100);
		goto out;
	}

	dev = fp_dev_open(ddev);
	fp_dscv_devs_free(discovered_devs);
	if (!dev) {
		printf("%d",-102);
		goto out;
	}

//	printf("Opened device. It's now time to enroll your finger.\n\n");
	data = enroll(dev);
	if (!data)
		goto out_close;
	printf("%d %d",909,minutiaCount);

//	r = fp_print_data_save(data, RIGHT_INDEX);
//	if (r < 0)
//		fprintf(stderr, "Data save failed, code %d\n", r);

//	fp_print_data_free(data);
out_close:
	fp_dev_close(dev);
out:
	fp_exit();
	return r;
}



/*
#             (C) 2020 Benjamin Kahn <xkahn@zoned.net>
#             Copyright (C) 2021 BELABOX project
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation; either version 2.1 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#define _GNU_SOURCE

#include <string.h>
#include <dlfcn.h>
#include <errno.h>
#include <linux/videodev2.h>

#define CAMLINK_NAME "Cam Link 4K"

int ioctl(int fd, int request, void *arg){
  /* We only care about device colorspace ioctls */
  if (request == (int)VIDIOC_ENUM_FMT) {
    struct v4l2_fmtdesc *fmt = (struct v4l2_fmtdesc *)arg;

    /* and only if it's a V4L2_BUF_TYPE_VIDEO_CAPTURE request */
    if (fmt->type == V4L2_BUF_TYPE_VIDEO_CAPTURE) {
      struct v4l2_capability cap;
      int ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);

      /* and only if the device is a camlink */
      if (ret == 0 &&
          strncmp((char *)cap.card, CAMLINK_NAME, strlen(CAMLINK_NAME)) == 0) {

        /* We only want to return the first colorspace format:
           NV12 at 4K and YUYV at lower resolutions */
        if (fmt->index > 0) {
          errno = EINVAL;
          return -1;
        }
      }
    }
  }

  /* Otherwise, execute the ioctl */
  static int (*orig_ioctl) (int, int, void *) = NULL;
  if (!orig_ioctl) {
    orig_ioctl = (int (*) (int, int, void *)) dlsym(RTLD_NEXT, "ioctl");
  }

  return orig_ioctl(fd, request, arg);
}

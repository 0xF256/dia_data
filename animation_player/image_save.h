#ifndef _IMAGE_SAVE_H_
#define _IMAGE_SAVE_H_

int image_save_png(char const *filename, int x, int y, int comp, const void *data, int stride_bytes);

#endif

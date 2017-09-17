#ifndef BMP_H
#define BMP_H

#ifdef __cplusplus
extern "C" {
#endif

int	 bmp_getHdr(void *bmp_data, int *w_p, int *h_p, int *bpp_p, int *clutNum_p);
int  bmp_getClut(void *bmp_data, void *clut, int clutNum);
int  bmp_read(void *bmp_data, void *dst, int wb, int h, int bpp, void *clut, int dir);
int  bmp_write(void *bmp_data, int w, int h, int bpp, void *src, int srcWb, int srcBpp, void *clut, int dir);
int  bmp_writeEx(void *bmp_data,int w, int h, int bpp, void *src, int srcWb, int srcBpp, void *clut, int clutNum, int dir);

#ifdef __cplusplus
};
#endif

#endif

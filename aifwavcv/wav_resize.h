#ifndef WAV_RESIZE_H
#define WAV_RESIZE_H
// バイリニアを利用した、wav のリサイズ
void wav_resize(int16_t dst[], unsigned dstLen, const int16_t src[], unsigned srcLen);
#endif

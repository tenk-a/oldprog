int RPAL_GetRGB(char far *rgb3x16); //ret tone
int RPAL_SetRGB(int tone, char far *rgb3x16);
int RPAL_GetTone(void);
int RPAL_SetTone(int tone);
int RPAL_Forget(void);
char far *RPAL_Search(void);
void RPAL_SetDfltPal(int tone);

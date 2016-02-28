int EMS_Exist(void);
int EMS_Free(unsigned hdl);
unsigned EMS_Alloc(int pagCnt); //ret handle
unsigned EMS_PageSeg(void);
unsigned EMS_Page(int hdl, int pag);
int EMS_FreePageCount(void);


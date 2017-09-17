/*--------------------------------------------------------------------
    D-Pixed ver.2.08 or	Later

    D-Pixed  Add-In  SDK

    	Header for Add-In

    1996-2000	Coypright(C) DOIchan!

    dp_add.h

------------------------------------------------------------------------
All Add-in tool	should include this header file.
-----------------------------------------------------------------------*/

#ifndef	_DPIXED_ADDIN_H__
#define	_DPIXED_ADDIN_H__

#ifndef	__WINDOWS_H
#include <windows.h>
#endif	//__WINDOWS_H

#ifdef __cplusplus
extern "C" {
#endif	//__cplusplus

//--------------------------------------------------------------------
//  information	for Decoder and	Encoder
//--------------------------------------------------------------------
typedef	struct _dp_encdecinfo__	{
    DWORD dwSize;
    DWORD dwFlag;   	    	    	//Flag for decoder/encoder
    int	keyColor;   	    	    	//Transparent color
    int	bgColor;    	    	    	//Background color
    DWORD nPicture; 	    	    	//Numbers of Pictures
    LPBITMAPINFOHEADER *pDibBuf;    	//DIB header buffer
    LPBYTE *pBitBuf;	    	    	//DIB bit data buffer
}DP_ENCDECINFO;

//flags
#define	DPDECENC_HANDLE	    	    0x00000001	//pDibBuf and pBitBuf are HANDLE not pointer
#define	DPDECENC_SEPARATED  	    0x00000002	//DIB header and bit data are not sequent

#define	DPDECENC_KEYCOLOR   	    0x00000010	//Enable keyColor
#define	DPDECENC_BACKGROUNDCOLOR    0x00000020	//Enable bgColor

//---------------------------------------------------------------------
//  	information for	Filter
//---------------------------------------------------------------------
typedef	struct _dp_filteraddin_struct {
    DWORD dwSize;
    LPBITMAPINFOHEADER pDib;	    //DIB Header
    BYTE *pBits;    	    	    //DIB bit data
    BYTE *pMaskBits;	    	    //Mask DIB bit data
    LPBITMAPINFOHEADER pRefDib;	    //Reference	DIB header
    BYTE *pRefBits; 	    	    //Reference	DIB bit	data
    DWORD dwFlag;   	    	    //flags
    HWND hwndDIB;   	    	    //HANDLE for DIB Window in D-Pixed
    int	leftColor;  	    	    //left selected palette in D-Pixed
    int	rightColor; 	    	    //right ...
    int	keyColor;   	    	    //transparent ...
    int	bgColor;    	    	    //bakcground ...
    int	mix;	    	    	    //0	- 16
    int	selectedSx; 	    	    //Selected rect
    int	selectedSy;
    int	selectedWidth;
    int	selectedHeight;
}DP_FILTERINFO;

//flags
#define	DPFILTER_MASKPLANE  	    0x00000001	    //Use maskplane
#define	DPFILTER_MASKON	    	    0x00000002	    //Enable mask
#define	DPFILTER_USEKEYCOLOR	    0x00000004	    //Enable transparent color
#define	DPFILTER_SELECTED   	    0x00000010	    //Area Selected
#define	DPFILTER_ENABLEREFERENCE    0x00000020	    //Enable reference DIB
#define	DPFILTER_CURRENTIMAGE	    0x00000100	    //Image is Current DIB

#define	DPFILTER_PALETTECHANGED	    0x00010000	    //palette changed

//-------------------------------------------------------
// Pen Add-In information
//-------------------------------------------------------

typedef	struct _dp_penaddin_struct {
    DWORD dwSize;
    LPBITMAPINFOHEADER pDib;
    BYTE *pBits;
    BYTE *pMaskBits;
    BYTE *pOffscreen;
    LPBITMAPINFOHEADER pPenDib;
    BYTE *pPenBits;
    DWORD dwFlag;
    int	leftColor;
    int	rightColor;
    int	keyColor;
    int	bgColor;
    int	mix;
    LONG x;
    LONG y;
    RECT redrawRect;
    HDC	hdc;
    HANDLE reserved;
    //---Ver.2.08-----
    BYTE *pAreaBits;
    RECT areaRect;
    RECT dragRect;
}DP_PENINFO;

//Flag for Pen Add-In
#define	DPWF_MASKON 	    	    0x00000010	    //Enable Mask
#define	DPWF_MASK   	    	    0x00000020	    //pBits is Mask plane
#define	DPWF_USETRANSPARENTCOLOR    0x00001000	    //Enable keyColor

#define	DPWF_WANTREDRAW	    	    0x03000000	    //Enable redrawRect	and redraw image
#define	DPWF_SETPALETTE	    	    0x04000000	    //Palette Select by	Add-In
#define	DPWF_SETMIX 	    	    0x08000000	    //Set Mix Value by Add-In
#define	DPWF_NOINVALIDATE   	    0x80000000	    //Only Blt Offscreen to Display

//Message for Pen Add-In

//  	Post from DIB Window
#define	DPWM_ACTIVATE	    	(WM_USER + 1)	    //DIB Window Activate
#define	DPWM_PALETTECHANGED 	(WM_USER + 2)	    //Palette Table Changed
#define	DPWM_SELECTPALETTE  	(WM_USER + 4)	    //User Select Palette
#define	DPWM_COMMANDCHANGED 	(WM_USER + 9)	    //User Select Other	Pen tool
#define	DPWM_COMMANDSELECTED	(WM_USER + 10)	    //User Select Pen tool

//Post from Pen	Add-In
#define	DPWM_DISPMESSAGE    	(WM_USER + 12)	    //Show Message on Status bar
#define	DPWM_DIB2SCREEN	    	(WM_USER + 30)	    //Compute DIB point	to Screen Point
#define	DPWM_SCREEN2DIB	    	(WM_USER + 31)	    //Compute Screen point to DIB point
#define	DPWM_UPDATEOFFSCREEN	(WM_USER + 32)	    //Update Offscreen buffer
#define	DPWM_BLTOFFSCREEN   	(WM_USER + 33)	    //Blit ofscreen to Display
#define	DPWM_GETINFO	    	(WM_USER + 34)	    //Get Pen Add-In Info
//ver.2.16
#define	DPWM_PALETTEUPDATE  	(WM_USER + 35)	    //palette changed by Add-In


//---------------------------------------------------------
//  information	of Add-in tool
//---------------------------------------------------------
typedef	struct _dp_addin_info__	{
    DWORD dwSize;
    DWORD dwType;   	    	//Add-In type
    LPSTR name;	    	    	//name that showed in menu
    LPSTR description;	    	//Encoder / Decoder File Description
    LPSTR defExt;   	    	//Encoder / Decoder File default extension
    LPSTR copyright;	    	//Copyright Message
    LPSTR version;  	    	//Version String
    LPSTR addinDescription; 	//Add-In Information
}DP_ADDININFO;

//Add-In types
#define	DPADDIN_ENCODER	    	    0x00002001	    //Encoder
#define	DPADDIN_DECODER	    	    0x00002002	    //Decoder
#define	DPADDIN_FILTER	    	    0x00002004	    //Filter
#define	DPADDIN_PEN 	    	    0x00002008	    //Pen

#define	DPADDIN_PEN_ENABLEMODE	    0x00010000	    //Drawing Mode Change
#define	DPADDIN_PEN_DISABLEFREEMODE 0x00020000	    //Free-Hand	Mode Not Supported
#define	DPADDIN_PEN_ENABLEPENMOVE   0x01000000	    //Enable Pen Outline

//--------------------------------------------------------------
//  Procedure definitions for Add-In tools
//--------------------------------------------------------------

//
//  Procedure for Decoder
BOOL DPDecode(LPCSTR,DP_ENCDECINFO *);
//
//  Procedure for Encoder
BOOL DPEncode(LPCSTR,DP_ENCDECINFO *);
//
//  Procedure for Filter
BOOL DPFilter(DP_FILTERINFO *);

BOOL DPFilterSetup(HWND);

//
//Procedure for	Pen
LONG DPPen(HWND,UINT,WPARAM,LPARAM,DP_PENINFO *);


//
//  Procedure to get information (Always this must exist)
BOOL DPGetInfo(DP_ADDININFO *);

#ifdef __cplusplus
}
#endif	//__cplusplus

#endif	//_DPIXED_ADDIN_H__


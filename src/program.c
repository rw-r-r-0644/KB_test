#include "program.h"
#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/vpad_functions.h"

struct key_state {
	unsigned char ch;
	unsigned char scancode;
	unsigned int state; //1=press; 0=release; 3=keep pressing
	char unknown[4]; //?
	unsigned short UTF16;
};

int pline;
void plog(char* txt) { //print log line to screen
	OSScreenPutFontEx(1, 0, pline, txt);
	flipBuffers();
	OSScreenPutFontEx(1, 0, pline, txt);
	flipBuffers();
	pline++;
	if (pline==16) {
		for (int a = 0; a < 2; a++) {
			fillScreen(0,0,0,0);
			flipBuffers();
		}
		pline=0;
	}
}
void kb_connection_callback(unsigned char *ch) {
	char buf3[255];
	__os_snprintf(buf3,255,"kb connected at channel %d",*ch);
	plog(buf3);
}
void kb_disconnection_callback(unsigned char *ch) {
	char buf4[255];
	__os_snprintf(buf4,255,"kb disconnected at channel %d",*ch);
	plog(buf4);
}
void kb_key_callback(struct key_state *key) {
	if(key->state==1) {
		char buf5[255];
		__os_snprintf(buf5,255,"kpress(%d) scode:%02x char: %c",key->ch, key->scancode, (char)key->UTF16);
		plog(buf5);
	}
}

int _entryPoint() {
	InitOSFunctionPointers();
	InitVPadFunctionPointers();
	OSScreenInit();
	int buf0_size = OSScreenGetBufferSizeEx(0);
	OSScreenSetBufferEx(0, (void *)0xF4000000);
	OSScreenSetBufferEx(1, (void *)0xF4000000 + buf0_size);
	int ii;
	for (ii = 0; ii < 2; ii++) {
		fillScreen(0,0,0,0);
		flipBuffers();
	}
	OSScreenEnableEx(0, 1);
	OSScreenEnableEx(1, 1);

	//nsyskbd.rpl
	unsigned int nsyskbd_handle;
	OSDynLoad_Acquire("nsyskbd.rpl", &nsyskbd_handle);

	char(*KBDSetup)(void *connection_callback, void *disconnection_callback, void* key_callback);
	char(*KBDTeardown)();	

	OSDynLoad_FindExport(nsyskbd_handle, 0, "KBDSetup", &KBDSetup);
	OSDynLoad_FindExport(nsyskbd_handle, 0, "KBDTeardown", &KBDTeardown);

	char kb_ret_value=KBDSetup(&kb_connection_callback,&kb_disconnection_callback,&kb_key_callback);

	char buf2[255];
	__os_snprintf(buf2,255,"KBDSetup called; ret: %d",kb_ret_value);
	plog(buf2);

	int error;
	VPADData vpad_data;
	while (1) {
		VPADRead(0, &vpad_data, 1, &error); //Get the status of the gamepad
		if (vpad_data.btns_h&VPAD_BUTTON_HOME) break; //To exit the game
	}

	char kbt_ret_value=KBDTeardown();
	char buf1[255];
	__os_snprintf(buf1,255,"KBDTeardown called; ret: %d",kbt_ret_value);
	plog(buf1);
	unsigned int t1 = 0x60000000;
	while(t1--) ;
	for(ii = 0;ii<2;ii++) {
		fillScreen(0,0,0,0);
		flipBuffers();
	}
	return 0;
}



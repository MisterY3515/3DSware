#include "3dsware/camera.h"

namespace Hardware {

Camera& Camera::getInstance() {
	static Camera instance;
	return instance;
}

Camera::Camera() : ready(false), camBuffer(nullptr), camBufferSize(0), width(320), height(240) {}

Camera::~Camera() {
	shutdown();
}

bool Camera::init() {
	if (ready) return true;

	Result res = camInit();
	if (R_FAILED(res)) {
		return false;
	}

	camBufferSize = width * height * 2; // RGB565
	camBuffer = (u16*)linearAlloc(camBufferSize);
	if (!camBuffer) {
		camExit();
		return false;
	}

	CAMU_SetSize(SELECT_OUT1, SIZE_QVGA, CONTEXT_A);
	CAMU_SetOutputFormat(SELECT_OUT1, OUTPUT_RGB_565, CONTEXT_A);
	CAMU_SetFrameRate(SELECT_OUT1, FRAME_RATE_30);
	CAMU_SetNoiseFilter(SELECT_OUT1, true);
	CAMU_SetAutoExposure(SELECT_OUT1, true);
	CAMU_SetAutoWhiteBalance(SELECT_OUT1, true);

	ready = true;
	return true;
}

void Camera::shutdown() {
	if (!ready) return;
	
	CAMU_Activate(SELECT_OUT1);
	CAMU_ClearBuffer(PORT_CAM1);
	
	if (camBuffer) {
		linearFree(camBuffer);
		camBuffer = nullptr;
	}
	
	camExit();
	ready = false;
}

bool Camera::captureFrame(u16* outBuffer) {
	if (!ready || !camBuffer) return false;

	u32 transferUnit = 0;
	CAMU_GetMaxBytes(&transferUnit, width, height);
	CAMU_SetTransferBytes(PORT_CAM1, transferUnit, width, height);

	CAMU_Activate(SELECT_OUT1);
	CAMU_ClearBuffer(PORT_CAM1);
	CAMU_StartCapture(PORT_CAM1);

	Handle sleepHandle;
	svcCreateTimer(&sleepHandle, RESET_ONESHOT);
	svcSetTimer(sleepHandle, 200000000LL, 0); // 200ms timeout

	bool captured = false;
	while (true) {
		u32 transfered = 0;
		Handle events[2] = { sleepHandle, 0 };
		s32 index = 0;
		svcWaitSynchronizationN(&index, events, 1, false, 1000000LL); // Check every 1ms
		
		CAMU_GetTransferBytes(&transfered, PORT_CAM1);

		if (transfered == camBufferSize) {
			captured = true;
			break;
		}
		
		// check timeout
		if (svcWaitSynchronization(sleepHandle, 0) == 0) {
			break;
		}
	}
	
	svcCloseHandle(sleepHandle);
	CAMU_StopCapture(PORT_CAM1);
	
	if (captured) {
		Handle camReceiveEvent = 0;
		Result res = CAMU_SetReceiving(&camReceiveEvent, camBuffer, PORT_CAM1, camBufferSize, (s16)(transferUnit & 0xFFFF));
		if (R_SUCCEEDED(res)) {
			svcWaitSynchronization(camReceiveEvent, 300000000LL);
			svcCloseHandle(camReceiveEvent);
			svcSleepThread(10000000); // 10ms wait to let DMA finish
			if (outBuffer) {
				memcpy(outBuffer, camBuffer, camBufferSize);
			}
			return true;
		}
	}
	
	return false;
}

u16* Camera::getInternalBuffer() const {
	return camBuffer;
}

bool Camera::captureToTexture(C3D_Tex* tex) {
	if (!ready || !camBuffer || !tex) return false;

	if (!captureFrame()) return false;

	// Flush the linear buffer so GPU sees it
	GSPGPU_FlushDataCache(camBuffer, camBufferSize);

	// DMA Transfer to tiled VRAM texture
	// Tex is 512x256 but we transfer 320x240
	GX_DisplayTransfer((u32*)camBuffer, GX_BUFFER_DIM(width, height),
	                   (u32*)tex->data, GX_BUFFER_DIM(512, 256),
	                   GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(1) | 
	                   GX_TRANSFER_RAW_COPY(0) | 
	                   GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGB565) | 
	                   GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB565) | 
	                   GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO));

	gspWaitForPPF();
	return true;
}

} // namespace Hardware

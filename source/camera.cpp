#include "3dsware/camera.h"

namespace Hardware {

Camera& Camera::getInstance() {
	static Camera instance;
	return instance;
}

Camera::Camera() : ready(false), camBuffer(nullptr), camBufferSize(0), width(320), height(240), currentPort(PORT_CAM1) {}

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

	u32 selectOut = (currentPort == PORT_CAM1) ? SELECT_OUT1 : SELECT_OUT2;
	CAMU_SetSize(selectOut, SIZE_QVGA, CONTEXT_A);
	CAMU_SetOutputFormat(selectOut, OUTPUT_RGB_565, CONTEXT_A);
	CAMU_SetFrameRate(selectOut, FRAME_RATE_30);
	CAMU_SetNoiseFilter(selectOut, true);
	CAMU_SetAutoExposure(selectOut, true);
	CAMU_SetAutoWhiteBalance(selectOut, true);

	ready = true;
	return true;
}

void Camera::shutdown() {
	if (!ready) return;
	
	u32 selectOut = (currentPort == PORT_CAM1) ? SELECT_OUT1 : SELECT_OUT2;
	CAMU_Activate(selectOut);
	CAMU_ClearBuffer(currentPort);
	
	if (camBuffer) {
		linearFree(camBuffer);
		camBuffer = nullptr;
	}
	
	camExit();
	ready = false;
}

bool Camera::captureFrame(u16* outBuffer) {
	if (!ready || !camBuffer) return false;

	u32 selectOut = (currentPort == PORT_CAM1) ? SELECT_OUT1 : SELECT_OUT2;

	u32 transferUnit = 0;
	CAMU_GetMaxBytes(&transferUnit, width, height);
	CAMU_SetTransferBytes(currentPort, transferUnit, width, height);

	CAMU_Activate(selectOut);
	CAMU_ClearBuffer(currentPort);

	// Set up receiving BEFORE starting capture
	Handle camReceiveEvent = 0;
	Result res = CAMU_SetReceiving(&camReceiveEvent, camBuffer, currentPort, camBufferSize, (s16)(transferUnit & 0xFFFF));
	if (R_FAILED(res)) {
		return false;
	}

	CAMU_StartCapture(currentPort);

	// Wait for the frame to arrive (500ms timeout)
	Result waitRes = svcWaitSynchronization(camReceiveEvent, 500000000LL);
	svcCloseHandle(camReceiveEvent);

	CAMU_StopCapture(currentPort);

	if (R_FAILED(waitRes)) {
		return false;
	}

	GSPGPU_FlushDataCache(camBuffer, camBufferSize);

	if (outBuffer) {
		memcpy(outBuffer, camBuffer, camBufferSize);
	}
	return true;
}

u16* Camera::getInternalBuffer() const {
	return camBuffer;
}

bool Camera::captureToTexture(C3D_Tex* tex) {
	if (!ready || !camBuffer || !tex) return false;

	if (!captureFrame()) return false;

	// GX_DisplayTransfer requires matching dimensions if scaling is disabled.
	// We must stage the 320x240 linear buffer into a 512x256 linear buffer.
	static u16* stageBuffer = nullptr;
	if (!stageBuffer) {
		stageBuffer = (u16*)linearAlloc(512 * 256 * 2);
		if (!stageBuffer) return false;
		memset(stageBuffer, 0, 512 * 256 * 2);
	}

	for (int y = 0; y < height; y++) {
		memcpy(stageBuffer + (y * 512), camBuffer + (y * width), width * 2);
	}

	GSPGPU_FlushDataCache(stageBuffer, 512 * 256 * 2);

	// DMA Transfer to tiled VRAM texture
	GX_DisplayTransfer((u32*)stageBuffer, GX_BUFFER_DIM(512, 256),
	                   (u32*)tex->data, GX_BUFFER_DIM(512, 256),
	                   GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(1) | 
	                   GX_TRANSFER_RAW_COPY(0) | 
	                   GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGB565) | 
	                   GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB565) | 
	                   GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO));

	gspWaitForPPF();
	return true;
}

void Camera::flipCamera() {
	if (!ready) return;
	currentPort = (currentPort == PORT_CAM1) ? PORT_CAM2 : PORT_CAM1;
	u32 selectOut = (currentPort == PORT_CAM1) ? SELECT_OUT1 : SELECT_OUT2;
	CAMU_SetSize(selectOut, SIZE_QVGA, CONTEXT_A);
	CAMU_SetOutputFormat(selectOut, OUTPUT_RGB_565, CONTEXT_A);
	CAMU_SetFrameRate(selectOut, FRAME_RATE_30);
	CAMU_SetNoiseFilter(selectOut, true);
	CAMU_SetAutoExposure(selectOut, true);
	CAMU_SetAutoWhiteBalance(selectOut, true);
}

} // namespace Hardware

#include "3dsware/mic.h"
#include <cstring>
#include <cstdio>

namespace Hardware {

Mic& Mic::getInstance() {
	static Mic instance;
	return instance;
}

Mic::Mic() : ready(false), streaming(false), micBuffer(nullptr), micBufferSize(0), lastMicPos(0), lastError(0) {}

Mic::~Mic() {
	shutdown();
}

bool Mic::init() {
	if (ready) return true;

	micBufferSize = 0x30000; // 192KB, as in official example
	micBuffer = (u8*)linearMemAlign(micBufferSize, 0x1000);
	if (!micBuffer) {
		lastError = 0xDEADBEEF; // Alloc error
		return false;
	}

	memset(micBuffer, 0, micBufferSize);
	Result res = micInit(micBuffer, micBufferSize);
	lastError = res;
	if (R_FAILED(res)) {
		linearFree(micBuffer);
		micBuffer = nullptr;
		return false;
	}

	ready = true;
	return true;
}

void Mic::shutdown() {
	if (!ready) return;
	
	stopStreaming();
	
	if (micBuffer) {
		micExit();
		linearFree(micBuffer);
		micBuffer = nullptr;
	}
	
	ready = false;
}

bool Mic::startStreaming() {
	if (streaming) return true;
	if (!ready || !micBuffer) return false;

	u32 audioSize = micBufferSize - 4;
	Result res = MICU_StartSampling(MICU_ENCODING_PCM16_SIGNED, MICU_SAMPLE_RATE_32730, 0, audioSize, true);
	if (R_FAILED(res)) return false;

	streaming = true;
	lastMicPos = 0;
	return true;
}

void Mic::stopStreaming() {
	if (!streaming) return;
	MICU_StopSampling();
	streaming = false;
}

bool Mic::hasNewSamples() const {
	if (!streaming || !ready) return false;
	const u32 limit = micBufferSize >= 4 ? micBufferSize - 4 : 0;
	if (limit == 0 || lastMicPos >= limit) return false;
	
	const u32 currentPos = micGetLastSampleOffset();
	if (currentPos >= limit) return false;
	
	return currentPos != lastMicPos;
}

size_t Mic::readSamples(int16_t* buffer, size_t maxSamples) {
	if (!buffer || maxSamples == 0 || !streaming || !ready) return 0;
	
	u32 limit = micBufferSize >= 4 ? micBufferSize - 4 : 0;
	if (limit == 0) return 0;
	
	if (lastMicPos >= limit) lastMicPos = 0;

	u32 currentPos = micGetLastSampleOffset();
	if (currentPos >= limit) return 0;
	if (currentPos == lastMicPos) return 0;
	
	u32 bytesAvailable;
	if (currentPos >= lastMicPos) {
		bytesAvailable = currentPos - lastMicPos;
	} else {
		bytesAvailable = limit - lastMicPos + currentPos;
	}
	
	size_t samplesAvailable = bytesAvailable / 2;
	if (samplesAvailable > maxSamples) samplesAvailable = maxSamples;
	
	size_t firstPart = limit - lastMicPos;
	size_t bytesToRead = samplesAvailable * 2;
	
	if (bytesToRead <= firstPart) {
		memcpy(buffer, micBuffer + lastMicPos, bytesToRead);
	} else {
		memcpy(buffer, micBuffer + lastMicPos, firstPart);
		memcpy((u8*)buffer + firstPart, micBuffer, bytesToRead - firstPart);
	}
	
	lastMicPos = (lastMicPos + bytesToRead) % limit;
	return samplesAvailable;
}

} // namespace Hardware

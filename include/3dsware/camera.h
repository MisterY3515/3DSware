#pragma once

#include <3ds.h>
#include <citro3d.h>

namespace Hardware {

class Camera {
public:
	static Camera& getInstance();

	/**
	 * Inizializza cam:u e alloca i buffer necessari.
	 * Ritorna true se il permesso viene ottenuto e l'hardware è pronto.
	 */
	bool init();

	/**
	 * Chiude la fotocamera e dealloca la memoria.
	 */
	void shutdown();

	/**
	 * Cattura un singolo frame e lo salva nel buffer lineare in formato RGB565.
	 * Se outBuffer è nullptr, viene salvato nel buffer interno.
	 */
	bool captureFrame(u16* outBuffer = nullptr);

	/**
	 * Restituisce l'ultimo buffer lineare acquisito internamente.
	 */
	u16* getInternalBuffer() const;

	/**
	 * Trasferisce il buffer lineare catturato direttamente in una texture VRAM
	 * usando il DMA (PICA200 GX_DisplayTransfer), swizzlando automaticamente.
	 * Ritorna true se completato con successo.
	 */
	bool captureToTexture(C3D_Tex* tex);

	bool isReady() const { return ready; }
	
	/**
	 * Scambia la telecamera tra esterna (PORT_CAM1) e interna (PORT_CAM2).
	 * Da chiamare mentre isReady() è true.
	 */
	void flipCamera();

private:
	Camera();
	~Camera();

	Camera(const Camera&) = delete;
	Camera& operator=(const Camera&) = delete;

	bool ready;
	u16* camBuffer;
	u32 camBufferSize;
	u16 width, height;
	u32 currentPort;
};

} // namespace Hardware

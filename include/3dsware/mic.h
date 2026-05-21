#pragma once

#include <3ds.h>
#include <vector>

namespace Hardware {

class Mic {
public:
	static Mic& getInstance();

	/**
	 * Inizializza il microfono e alloca il ring buffer.
	 * @return true se l'inizializzazione ha avuto successo.
	 */
	bool init();

	/**
	 * Dealloca e spegne il microfono.
	 */
	void shutdown();

	/**
	 * Inizia la cattura continua del microfono (streaming).
	 * Utilizzato per app Voice Chat / in tempo reale.
	 */
	bool startStreaming();

	/**
	 * Ferma la cattura continua.
	 */
	void stopStreaming();

	/**
	 * Legge un blocco di sample dal ring buffer hardware.
	 * Da chiamare frequentemente se startStreaming() è attivo.
	 * @param outBuffer Array in cui salvare i sample.
	 * @param maxSamples Massimo numero di sample da leggere.
	 * @return Numero di sample effettivamente letti.
	 */
	size_t readSamples(int16_t* outBuffer, size_t maxSamples);

	/**
	 * Ritorna true se ci sono nuovi dati audio disponibili da leggere.
	 */
	bool hasNewSamples() const;

	bool isReady() const { return ready; }
	bool isStreaming() const { return streaming; }

private:
	Mic();
	~Mic();

	Mic(const Mic&) = delete;
	Mic& operator=(const Mic&) = delete;

	bool ready;
	bool streaming;
	
	u8* micBuffer;
	u32 micBufferSize;
	u32 lastMicPos;
};

} // namespace Hardware

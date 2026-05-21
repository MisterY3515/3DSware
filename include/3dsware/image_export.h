#pragma once

#include <3ds.h>

namespace Hardware {

class ImageExport {
public:
	/**
	 * Salva un buffer RGB565 lineare in un file BMP RGB888 puro.
	 * Gestisce automaticamente il padding dei byte e l'orientamento bottom-up.
	 * @param path Il percorso in cui salvare (es. "sdmc:/foto.bmp").
	 * @param rgb565 Il buffer dell'immagine.
	 * @param width Larghezza immagine.
	 * @param height Altezza immagine.
	 * @return true se il salvataggio è andato a buon fine.
	 */
	static bool saveBMP(const char* path, const u16* rgb565, int width, int height);

	// TODO: savePNG e saveJPG verranno implementati con stb_image_write
	// in un modulo separato a richiesta.
};

} // namespace Hardware

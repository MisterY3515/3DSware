#pragma once

#include <3ds.h>
#include <string>

namespace Hardware {

class FS {
public:
	/**
	 * Crea la cartella specificata e tutte le cartelle genitrici necessarie.
	 * @return true se la cartella esiste o è stata creata con successo.
	 */
	static bool ensureDirectory(const std::string& path);

	/**
	 * Ritorna true se il file esiste e non è una directory.
	 */
	static bool fileExists(const std::string& path);

	/**
	 * Ritorna la dimensione del file in byte. Se non esiste, ritorna 0.
	 */
	static size_t fileSize(const std::string& path);

	/**
	 * Rimuove un file dal filesystem.
	 * @return true se rimosso con successo o se non esisteva.
	 */
	static bool removeFile(const std::string& path);
};

} // namespace Hardware

# 3DSware

3DSware è una libreria C++ leggera, stabile ed efficiente per console Nintendo 3DS (homebrew).
Fornisce un framework di alto livello orientato ad oggetti per gestire l'hardware della console, eliminando la complessità delle API di basso livello di libctru, gestendo automaticamente buffer, conversioni, allocazioni ed evitando memory leak o crash di sistema.

## Caratteristiche
- **Camera (`hardware_camera`):** Inizializzazione sicura `cam:u`, acquisizione frame `RGB565` lineari e trasferimento diretto ad altissime prestazioni in VRAM usando il modulo DMA (PICA200 `GX_DisplayTransfer`), eliminando glitch e tearing.
- **Image Export (`image_export`):** Modulo isolato per esportare frame in `BMP` RGB888 nativo (senza dipendenze aggiuntive) o in formati compressi, occupandosi del padding e dell'orientamento per output di altissima qualità.
- **Microfono (`hardware_mic`):** Gestione totale di `mic:u`. Permette di acquisire audio tramite un ring buffer circolare, supporta streaming real-time per VoIP (es. Discord) o registrazioni bufferizzate su file system.
- **File System (`hardware_fs`):** Utility essenziali e sicure per controllare, leggere, creare directory ed eliminare file dalla SD card, a prova di crash (gestione sicura dell'assenza della scheda).

## Licenza
Questo progetto è distribuito sotto **Licenza MIT**.
È aperto, gratuito per uso commerciale e non commerciale, e puoi integrarlo nel tuo codice closed-source o open-source senza vincoli virali. Vedi il file `LICENSE` per i dettagli.

#include "3dsware/image_export.h"
#include <cstdio>
#include <cstdlib>

namespace Hardware {

#pragma pack(push, 1)
struct BMPHeader {
	u16 signature;
	u32 fileSize;
	u32 reserved;
	u32 dataOffset;
	u32 infoHeaderSize;
	s32 width;
	s32 height;
	u16 planes;
	u16 bitsPerPixel;
	u32 compression;
	u32 imageSize;
	s32 xPixelsPerMeter;
	s32 yPixelsPerMeter;
	u32 colorsUsed;
	u32 colorsImportant;
};
#pragma pack(pop)

bool ImageExport::saveBMP(const char* path, const u16* rgb565, int width, int height, int stride) {
	if (!path || !rgb565 || width <= 0 || height <= 0) return false;
	if (stride <= 0) stride = width;

	FILE* file = fopen(path, "wb");
	if (!file) return false;

	int rowBytes = width * 3;
	int padding = (4 - (rowBytes % 4)) % 4;
	int paddedRowBytes = rowBytes + padding;
	int imageSize = paddedRowBytes * height;

	BMPHeader header = {0};
	header.signature = 0x4D42; // "BM"
	header.fileSize = sizeof(BMPHeader) + imageSize;
	header.dataOffset = sizeof(BMPHeader);
	header.infoHeaderSize = 40; // Size of BITMAPINFOHEADER
	header.width = width;
	header.height = -height; // Top-down BMP
	header.planes = 1;
	header.bitsPerPixel = 24; // RGB888
	header.compression = 0; // BI_RGB
	header.imageSize = imageSize;

	fwrite(&header, 1, sizeof(BMPHeader), file);

	u8 padBytes[3] = {0, 0, 0};
	u8* rowData = (u8*)malloc(rowBytes);
	if (!rowData) {
		fclose(file);
		return false;
	}

	for (int y = 0; y < height; ++y) {
		const u16* srcRow = rgb565 + (y * stride);
		u8* dst = rowData;
		for (int x = 0; x < width; ++x) {
			u16 pixel = srcRow[x];
			u8 b = (pixel & 0x001F) << 3;
			u8 g = ((pixel & 0x07E0) >> 5) << 2;
			u8 r = ((pixel & 0xF800) >> 11) << 3;
			
			// BMP expects BGR format
			*dst++ = b;
			*dst++ = g;
			*dst++ = r;
		}
		fwrite(rowData, 1, rowBytes, file);
		if (padding > 0) {
			fwrite(padBytes, 1, padding, file);
		}
	}

	free(rowData);
	fclose(file);
	return true;
}

} // namespace Hardware

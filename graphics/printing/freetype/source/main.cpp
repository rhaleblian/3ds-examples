/*
	Hello World example made by Aurelio Mannara for libctru
	This code was modified for the last time on: 12/13/2014 01:00 UTC+1

	This wouldn't be possible without the amazing work done by:
	-Smealum
	-fincs
	-WinterMute
	-yellows8
	-plutoo
	-mtheall
	-Many others who worked on 3DS and I'm surely forgetting about
*/

#include <stdio.h>
#include <string.h>

#include <3ds.h>
#include <3ds/result.h>
#include <3ds/services/fs.h>

#include <ft2build.h>
#include FT_FREETYPE_H

//This include a header containing definitions of our image
// #include "brew_bgr.h"

int loop(void) {
	// Main loop
	while (aptMainLoop())
	{
		//Scan all the inputs. This should be done once for each frame
		hidScanInput();

		//hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
		u32 kDown = hidKeysDown();

		if (kDown & KEY_START) break; // break in order to return to hbmenu

		// Flush and swap framebuffers
		gfxFlushBuffers();
		gfxSwapBuffers();

		//Wait for VBlank
		gspWaitForVBlank();
	}

	// Exit services
	gfxExit();

	return 0;
}

int assert(Result result, const char *message) {
	if (R_FAILED(result)) {
		printf("[FAIL] %s (%li)\n", message, result);
		return loop();
	}
	return 0;
}

void listFonts(void) {
	Result result;

	FS_Archive archive;
	result = FSUSER_OpenArchive(&archive, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, ""));
	const char *fontpath = "/Font";
	FS_Path fs_path = fsMakePath(PATH_ASCII, fontpath);
	printf("[ OK ] fsMakePath %s len=%lu\n", fontpath, fs_path.size);
	
	Handle handle;
	result = FSUSER_OpenDirectory(&handle, archive, fs_path);
	assert(result, "FSUSER_OpenDirectory");
	printf("[ OK ] FSUSER_OpenDirectory (%li) handle=%lu\n", result, handle);

	FS_DirectoryEntry entries[32];
	u32 count;
	result = FSDIR_Read(handle, &count, 32, entries);
	assert(result, "reading directory");
	printf("[ OK ] read %ld entries\n", count);
	for (u32 i = 0; i < count; i++) {
		printf("[ OK ] %x%x %s.%s\n",
			entries[i].name[0],
			entries[i].name[1],
			entries[i].shortName,
			entries[i].shortExt);
	}

	FSDIR_Close(handle);
}

int ftAssert(FT_Error error, const char *mkessage) {
	if (error) {
		printf("[FAIL] %s (%i)\n", message, error);
		return loop();
	}
	return 0;
}

void ftInitDefault(FT_Byte* file_base, u32 file_size) {
	FT_Library library;
	FT_Face face;
	FT_Error error;

	error = FT_Init_FreeType(&library);
	ftAssert(error, "FT_Init_FreeType");
	printf("[ OK ] FT_Init_FreeType (%i)\n", error);

	// error = FT_New_Face(library, fontpath, 0, &face);
	// ftAssert(error, "FT_New_Face");
	// printf("[ OK ] FT_New_Face (%i)\n", error);

	error = FT_New_Memory_Face(library, file_base, file_size, 0, &face);
	ftAssert(error, "FT_New_Face");
	printf("[ OK ] FT_New_Face (%i)\n", error);

	error = FT_Set_Char_Size(face, 0, 16 * 64, 300, 300);
	ftAssert(error, "FT_Set_Char_Size");
	printf("[ OK ] FT_Set_Char_Size (%i)\n", error);

	FT_GlyphSlot slot = face->glyph;

	FT_UInt glyph_index = FT_Get_Char_Index(face, 'A');
	error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
	ftAssert(error, "FT_Load_Glyph");
	printf("[ OK ] FT_Load_Glyph (%i)\n", error);

	error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
	ftAssert(error, "FT_Render_Glyph");
	printf("[ OK ] FT_Render_Glyph (%i)\n", error);

	printf("bitmap width=%i\n", slot->bitmap.width);
	printf("bitmap rows=%i\n", slot->bitmap.rows);
	printf("bitmap pitch=%i\n", slot->bitmap.pitch);
	printf("bitmap pixel_mode=%i\n", slot->bitmap.pixel_mode);
	printf("bitmap num_grays=%i\n", slot->bitmap.num_grays);
	printf("bitmap palette_mode=%i\n", slot->bitmap.palette_mode);

	// FT_Bitmap bitmap = slot->bitmap;

	// for (u8 y = 0; y < bitmap.rows; y++) {
	// 	for (u8 x = 0; x < bitmap.width; x++) {
	// 		u8 pixel = bitmap.buffer[y * bitmap.pitch + x];
	// 		printf("%02X ", pixel);
	// 	}
	// 	printf("\n");
	// }

	FT_Done_Face(face);
	FT_Done_FreeType(library);
}

void loadFont(void) {
	Result result;
	FS_Archive archive;

	result = fsInit();	
	assert(result, "fsInit");
	printf("[ OK ] fsInit (%li)\n", result);
	result = FSUSER_OpenArchive(&archive, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, ""));
	assert(result, "opening SDMC");
	printf("[ OK ] opening SDMC (%li)\n", result);

	const char *fontpath = "/font/LiberationSerif-Regular.ttf";
	Handle handle;
	result = FSUSER_OpenFile(&handle, archive, fsMakePath(PATH_ASCII, fontpath), FS_OPEN_READ, 0);
	assert(result, "opening font");
	printf("[ OK ] opening font (%li)\n"
		   "[ OK ] %s\n"
		   "[ OK ] handle=%lu\n", result, fontpath, handle);

	u64 size;
	result = FSFILE_GetSize(handle, &size);
	printf("[ OK ] %llu bytes\n", size);

	FT_Byte* buffer = new FT_Byte[size];
	u32 bytesRead;
	result = FSFILE_Read(handle, &bytesRead, 0, buffer, size);

	ftInitDefault(buffer, size);

	fsExit();
}

int main(int argc, char **argv)
{
	gfxInitDefault();

	//Initialize console on top screen. Using NULL as the second argument tells the console library to use the internal console structure as current one
	consoleInit(GFX_TOP, NULL);
	printf("\n");
	printf("[ OK ] dslibrOS\n");

	listFonts();

	//Load the font from SDMC
	// loadFont();
	// printf("[ OK ] font loaded\n");

	//We don't need double buffering in this example. In this way we can draw our image only once on screen.
	gfxSetDoubleBuffering(GFX_BOTTOM, false);

	//Get the bottom screen's frame buffer
	u16 width, height;
	u8* fb = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, &width, &height);
	auto format = gfxGetScreenFormat(GFX_BOTTOM);
	printf("[ OK ] %dx%d framebuffer format=%d\n", width, height, format);

	// size_t len = width * height * 3;
	// u8 canvas[len];
	// Copy our image in the bottom screen's frame buffer
	// memset(canvas, 0, len);
	// memcpy(fb, canvas, len);

	for (u8 i=0; i<height; i++) {
		for (u8 j=0; j<width; j++) {
			fb[(i * width + j) * 3 + 0] = 0xFF; // B
			fb[(i * width + j) * 3 + 1] = 0x00; // G
			fb[(i * width + j) * 3 + 2] = 0xFF; // R
		}
	}

	gfxFlushBuffers();

	return loop();
}

#include "texture.h"
#include <string>
#include "png.h"
#include <stdio.h>
#include "jpeglib.h"
#include "glextensionfuncs.h"
#include <assert.h>

// reference counter for resource management
static std::map<GLuint, unsigned short> RefCount;

Texture::Texture(const char *filename, bool mipmap) : id_(0), dims_()
{
	//~ printf("loading texture image from %s\n", filename); fflush(stdout);
	if (init(filename)) {
		printf("successfully loaded texture %s\n", filename); fflush(stdout);
		assert(RefCount.count(id_) == 0);
		RefCount[id_] = 1;
		assert(RefCount[id_] == 1);
		assert(RefCount.count(id_) == 1);
		if (mipmap) {
			glBindTexture(GL_TEXTURE_2D, id_);
			glGenerateMipmap(GL_TEXTURE_2D);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	} else {
		assert(0);
	}
}

Texture::Texture(const Texture &other) : id_(other.id_), dims_(other.dims_)
{
	assert(RefCount.count(id_) == 1);
	RefCount[id_]++;
}


Texture::~Texture()
{
	assert(RefCount.count(id_) == 1);
	assert(RefCount[id_] > 0);
	if (--RefCount[id_] == 0) {
		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteTextures(1, &id_);
	}
}

void Texture::bind(GLenum target)
{
	glBindTexture(target, id_);
}

void Texture::bindToImage(GLuint unit, GLenum access, GLenum format, GLuint level)
{
	glBindImageTexture(unit, id_, level, false, 0, access, format);
}

GLuint Texture::getID()
{
	return id_;
}

bool Texture::init(const char *filename)
{
	std::string fn(filename);
	std::string ext = fn.substr(fn.find_last_of(".")+1);
	if (ext == "png") {
		return loadPng(filename);
	} else if (ext == "jpg" || ext == "jpeg") {
		return loadJpg(filename);
	} else {
		return false;
	}
}

// load png code from http://afsharious.wordpress.com/2011/06/30/loading-transparent-pngs-in-opengl-for-dummies/
bool Texture::loadPng(const char *filename)
{
	//~ printf("loading png %s\n", filename); fflush(stdout);
	png_byte header[8];
	FILE *file = fopen(filename, "rb"); // open as binary
	if (!file) {
		//~ printf("couldn't get file pointer\n"); fflush(stdout);
		return false;
	}
	fread(header, 1, 8, file);
	bool is_png = !png_sig_cmp(header, 0, 8);
	if (!is_png) {
		//~ printf("file isn't png\n"); fflush(stdout);
		fclose(file);
		return false;
	}
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		//~ printf("couldn't get png pointer\n"); fflush(stdout);
		fclose(file);
		return false;
	}
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		//~ printf("couldn't get info pointer\n"); fflush(stdout);
		png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
		fclose(file);
		return false;
	}
	if (setjmp(png_jmpbuf(png_ptr))) {
		//~ printf("couldn't set jump function\n"); fflush(stdout);
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
		fclose(file);
		return false;
	}
	png_init_io(png_ptr, file);
	png_set_sig_bytes(png_ptr, 8);
	png_read_info(png_ptr, info_ptr);
	int bit_depth, color_type;
	png_uint_32 twidth, theight;
	png_get_IHDR(png_ptr, info_ptr, &twidth, &theight, &bit_depth, &color_type, NULL, NULL, NULL);
	dims_.x = twidth;
	dims_.y = theight;
	png_read_update_info(png_ptr, info_ptr);
	int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
	png_byte *data = new png_byte[rowbytes * theight];
	png_bytep *row_pointers = new png_bytep[theight];
	for (int i = 0; i < theight; i++) {
		row_pointers[theight - 1 - i] = data + i * rowbytes;
	}
	png_read_image(png_ptr, row_pointers);
	
	//sendGL(data);
	glGenTextures(1, &id_);
	//~ printf("generated texture id %u\n", id_); fflush(stdout);
	glBindTexture(GL_TEXTURE_2D, id_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dims_.x, dims_.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
	delete[] data;
	delete[] row_pointers;
	fclose(file);
	return true;
}

// loading jpeg code from http://mattikariluoma.com/?p=36
bool Texture::loadJpg(const char *filename)
{
	FILE *file = fopen(filename, "rb");
	if(!file) {
		return false;
	}
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, file);
	jpeg_read_header(&cinfo, 0);
	cinfo.scale_num = 1;
	cinfo.scale_denom = 1;
	jpeg_start_decompress(&cinfo);
	dims_.x = cinfo.output_width;
	dims_.y = cinfo.output_height;
	// cinfo.num_components should always be 3
	unsigned char *data = new unsigned char[dims_.x * dims_.y * 3];
	JSAMPROW row_pointer = new JSAMPLE[dims_.x * 3];
	unsigned long location = 0;
	while(cinfo.output_scanline < cinfo.output_height) {
		jpeg_read_scanlines(&cinfo, &row_pointer, 1);
		for(int i = 0; i < dims_.x * 3; i++) {
			data[location++] = row_pointer[i];
		}
	}
	
	//sendGL(data);
	glGenTextures(1, &id_);
	glBindTexture(GL_TEXTURE_2D, id_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, dims_.x, dims_.y, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	delete[] data;
	delete[] row_pointer;
	fclose(file);
	return true;
}

void Texture::sendGL(const GLvoid *data)
{
	glGenTextures(1, &id_);
	glBindTexture(GL_TEXTURE_2D, id_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dims_.x, dims_.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
}
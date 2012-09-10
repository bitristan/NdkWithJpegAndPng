#include "com_renren_image_process_demo_ImageProcessActivity.h"
#include <stdlib.h>
#include <stdio.h>
#include <jpeglib.h>
#include <png.h>
#include <setjmp.h>
#include <android/log.h>

#define LOG_TAG = "rr_image_process";

struct my_error_mgr {
	struct jpeg_error_mgr pub;
	jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void) my_error_exit(j_common_ptr cinfo) {
	my_error_ptr myerr = (my_error_ptr) cinfo->err;
	longjmp(myerr->setjmp_buffer, 1);
}

char* js2c(JNIEnv* env, jstring jstr) {
	char* rtn = NULL;
	jclass clsstring = (*env)->FindClass(env, "java/lang/String");
	jstring strencode = (*env)->NewStringUTF(env, "utf-8");
	jmethodID mid = (*env)->GetMethodID(env, clsstring, "getBytes",
			"(Ljava/lang/String;)[B");
	jbyteArray barr = (jbyteArray)(*env)->CallObjectMethod(env, jstr, mid,
			strencode);
	jsize alen = (*env)->GetArrayLength(env, barr);
	jbyte* ba = (*env)->GetByteArrayElements(env, barr, JNI_FALSE);
	if (alen > 0) {
		rtn = (char*) malloc(alen + 1);
		memcpy(rtn, ba, alen);
		rtn[alen] = 0;
	}
	(*env)->ReleaseByteArrayElements(env, barr, ba, 0);
	return rtn;
}

void process_jpeg(FILE *in_file, FILE *out_file) {
	if (in_file == NULL || out_file == NULL) {
		return;
	}

	fseek(in_file, 0L, SEEK_SET);

	struct jpeg_decompress_struct in;
	struct my_error_mgr in_err;

	in.err = jpeg_std_error(&in_err.pub);
	in_err.pub.error_exit = my_error_exit;
	if (setjmp(in_err.setjmp_buffer)) {
		jpeg_destroy_decompress(&in);
		return;
	}

	jpeg_create_decompress(&in);
	jpeg_stdio_src(&in, in_file);
	jpeg_read_header(&in, TRUE);

	jpeg_start_decompress(&in);

	int width = in.image_width;
	int height = in.image_height;
	int bytesPerPixel = in.num_components;

	struct jpeg_compress_struct out;
	struct my_error_mgr out_err;

	out.err = jpeg_std_error(&out_err.pub);
	out_err.pub.error_exit = my_error_exit;
	if (setjmp(out_err.setjmp_buffer)) {
		jpeg_destroy_compress(&out);
		return;
	}

	jpeg_create_compress(&out);
	jpeg_stdio_dest(&out, out_file);

	out.image_width = width;
	out.image_height = height;
	out.input_components = bytesPerPixel;
	out.in_color_space = JCS_RGB;

	jpeg_set_defaults(&out);
	jpeg_start_compress(&out, TRUE);

	JSAMPROW row_pointer[1];
	row_pointer[0] = (unsigned char *) malloc(width * bytesPerPixel);

	while (in.output_scanline < height) {
		jpeg_read_scanlines(&in, row_pointer, 1);
		jpeg_write_scanlines(&out, row_pointer, 1);
	}

	free(row_pointer[0]);

	jpeg_finish_decompress(&in);
	jpeg_destroy_decompress(&in);

	jpeg_finish_compress(&out);
	jpeg_destroy_compress(&out);
}

void process_png(FILE *in_file, FILE *out_file) {
	if (in_file == NULL || out_file == NULL) {
		return;
	}

	fseek(in_file, 0L, SEEK_SET);

	png_structp in_png_ptr;
	png_infop in_info_ptr;

	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type;

	in_png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL,
			NULL);

	if (in_png_ptr == NULL) {
		return;
	}

	in_info_ptr = png_create_info_struct(in_png_ptr);
	if (in_info_ptr == NULL) {
		png_destroy_read_struct(&in_png_ptr, NULL, NULL);
		return;
	}

	if (setjmp(png_jmpbuf(in_png_ptr))) {
		png_destroy_read_struct(&in_png_ptr, &in_info_ptr, NULL);
		return;
	}

	png_init_io(in_png_ptr, in_file);
	png_read_info(in_png_ptr, in_info_ptr);
	png_get_IHDR(in_png_ptr, in_info_ptr, &width, &height, &bit_depth,
			&color_type, &interlace_type, NULL, NULL);
	png_read_update_info(in_png_ptr, in_info_ptr);

	png_structp out_png_ptr;
	png_infop out_info_ptr;

	out_png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL,
			NULL);
	if (out_png_ptr == NULL) {
		return;
	}

	out_info_ptr = png_create_info_struct(out_png_ptr);
	if (out_info_ptr == NULL) {
		return;
	}

	if (setjmp(png_jmpbuf(out_png_ptr))) {
		png_destroy_write_struct(&out_png_ptr, &out_info_ptr);
		return;
	}

	png_init_io(out_png_ptr, out_file);
	png_set_IHDR(out_png_ptr, out_info_ptr, width, height, bit_depth,
			color_type, interlace_type, PNG_COMPRESSION_TYPE_BASE,
			PNG_FILTER_TYPE_BASE);

	if (setjmp(png_jmpbuf(out_png_ptr))) {
		png_destroy_write_struct(&out_png_ptr, &out_info_ptr);
		return;
	}
	png_write_info(out_png_ptr, out_info_ptr);
	if (setjmp(png_jmpbuf(out_png_ptr))) {
		png_destroy_write_struct(&out_png_ptr, &out_info_ptr);
		return;
	}
	// read and write image data
	png_byte *row_pointer = (png_byte *) malloc(
			png_get_rowbytes(in_png_ptr, in_info_ptr));
	int i, j;
	int loops = interlace_type == 0 ? 1 : interlace_type;
	for (i = 0; i < loops; i++) {
		for (j = 0; j < height; j++) {
			png_read_rows(in_png_ptr, &row_pointer, NULL, 1);
			png_write_rows(out_png_ptr, &row_pointer, 1);
		}
	}

	if (setjmp(png_jmpbuf(out_png_ptr))) {
		png_destroy_write_struct(&out_png_ptr, &out_info_ptr);
		return;
	}
	png_write_end(out_png_ptr, NULL);

	free(row_pointer);

	png_destroy_read_struct(&in_png_ptr, &in_info_ptr, NULL);
	png_destroy_write_struct(&out_png_ptr, &out_info_ptr);
}

int check_if_png(FILE *fp) {
	if (fp == NULL) {
		return 0;
	}

	const int SIZE = 4;
	char buf[SIZE];

	fseek(fp, 0L, SEEK_SET);

	if (fread(buf, sizeof(char), SIZE, fp) != SIZE) {
		return 0;
	}

	return (!png_sig_cmp(buf, (png_size_t) 0, SIZE));
}

JNIEXPORT jstring JNICALL Java_com_renren_image_process_demo_ImageProcessActivity_testLib(
		JNIEnv *env, jobject object) {
	return (*env)->NewStringUTF(env, "Hello, Jni world!");
}

JNIEXPORT void JNICALL Java_com_renren_image_process_demo_ImageProcessActivity_processImage(
		JNIEnv *env, jobject object, jstring j_in_file_name, jstring j_out_file_name) {
	FILE *in_file, *out_file;
	const char *in_file_name = js2c(env, j_in_file_name);
	if ((in_file = fopen(in_file_name, "rb")) == NULL) {
		return;
	}

	const char *out_file_name = js2c(env, j_out_file_name);
	if ((out_file = fopen(out_file_name, "wb")) == NULL) {
		return;
	}

	if (check_if_png(in_file)) {
		// process as png
		process_png(in_file, out_file);
	} else {
		// process as jpeg
		process_jpeg(in_file, out_file);
	}

	fclose(in_file);
	fclose(out_file);
}

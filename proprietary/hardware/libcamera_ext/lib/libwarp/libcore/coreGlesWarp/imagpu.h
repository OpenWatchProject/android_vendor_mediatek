#ifndef _VFB_IMAGPU_H_
#define _VFB_IMAGPU_H_
//-------------------------------------------------------------------------------
//-- Title        : The imaGPU library header
//
//-- Creator    : Eddie Tsao
//
//-- Version    : v0
//
//-- Description : imaGPU is a GLES2.0 base framework for image processing
//
//-------------------------------------------------------------------------------
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

// Index to bind the attributes to vertex shaders
#define VERTEX_ARRAY    0
#define TEXTURE_ARRAY    1
#define REF_ARRAY        2

// ImaGPU YUV planar format definition
#define IG_YUV422P        0x8000
#define IG_YUV420P        0x8001
#define IG_YUV420SP     0x8002

#define IG_API extern
//-------------------------------------------------------------------------------
//filename    : for shader file
//type        : GL_VERTEX_SHADER, GL_FRAGMENT_SHADER
//Return    : the shader handle id
IG_API GLuint igLoadShader( const char * filename, GLenum type );
//-------------------------------------------------------------------------------
//vs        : vertex shader handle
//fs        : fragment shader handle
//ref_buffer: reference buffer for vpg_ref. Set 0 if there is no ref_buffer
//Return    : the program handle id
IG_API GLuint igCreateProgram(GLuint vs, GLuint fs, GLfloat * ref_buffer );
//-------------------------------------------------------------------------------
//filename    : for ppm image file
//mode        : GL_REPEAT, GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT
//Return    : the image handle id
//IG_API GLuint igLoadImage( const char * filename, GLint mode );
//-------------------------------------------------------------------------------
//filename    : for raw RGB image file
//mode        : GL_REPEAT, GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT
//width        : image's width
//height    : image's height
//Return    : the image handle id
//IG_API GLuint igLoadImage( const char * filename, GLint mode , int width, int height );
//-------------------------------------------------------------------------------
//data        : for RGB image data pointer
//mode        : GL_REPEAT, GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT
//width        : image's width
//height    : image's height
//Return    : the image handle id
//IG_API GLuint igLoadImage( void * data, GLint mode , int width, int height);
//-------------------------------------------------------------------------------
//data        : for image data pointer
//mode        : GL_REPEAT, GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT
//format    : GL_RGB, GL_RGBA
//width        : image's width
//height    : image's height
//Return    : the image handle id
//IG_API GLuint igLoadImage( void * data, GLint mode, GLenum format, int width, int height);
//-------------------------------------------------------------------------------
//data        : for planar image data pointer
//mode        : GL_REPEAT, GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT
//format    : IG_YUV422P, IG_YUV420P, IG_YUV420SP
//width        : image's width
//height    : image's height
//plan        : the image plan handle pointer
//Return    : the image plan number
//IG_API GLuint igLoadImage(void * data, GLint mode, GLenum format, int width, int height, GLuint *plan);
//-------------------------------------------------------------------------------
//filename    : for ppm image file
//mode        : GL_REPEAT, GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT
//xoffset    : the x dim start point reference to pixel (0,0)
//yoffset    : the y dim start point reference to pixel (0,0)
//swidth    : Sub image's width
//sheight    : Sub image's height
//Return    : the image handle id
IG_API GLuint igLoadSubImage( const char * filename, GLint mode, GLint xoffset, GLint yoffset, GLint swidth, GLint sheight);
//-------------------------------------------------------------------------------
//filename    : for raw RGB image file
//mode        : GL_REPEAT, GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT
//width        : image's width
//height    : image's height
//xoffset    : the x dim start point reference to pixel (0,0)
//yoffset    : the y dim start point reference to pixel (0,0)
//swidth    : Sub image's width
//sheight    : Sub image's height
//Return    : the image handle id
IG_API GLuint igLoadSubImage( const char * filename, GLint mode , GLint width, GLint height, GLint xoffset, GLint yoffset, GLint swidth, GLint sheight);
//-------------------------------------------------------------------------------
//data        : for image data pointer
//mode        : GL_REPEAT, GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT
//format    : GL_RGB, GL_RGBA
//width        : image's width
//height    : image's height
//xoffset    : the x dim start point reference to pixel (0,0)
//yoffset    : the y dim start point reference to pixel (0,0)
//swidth    : Sub image's width
//sheight    : Sub image's height
//Return    : the image handle id
IG_API GLuint igLoadSubImage( void * data, GLint mode, GLenum format, int width, int height, GLint xoffset, GLint yoffset, GLint swidth, GLint sheight);
//-------------------------------------------------------------------------------
//data        : for planar image data pointer
//mode        : GL_REPEAT, GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT
//format    : IG_YUV422P, IG_YUV420P, IG_YUV420SP
//width        : image's width
//height    : image's height
//xoffset    : the x dim start point reference to pixel (0,0)
//yoffset    : the y dim start point reference to pixel (0,0)
//swidth    : Sub image's width
//sheight    : Sub image's height
//plan        : the image plan handle pointer
//Return    : the image plan number
IG_API GLuint igLoadSubImage(void * data, GLint mode, GLenum format, int width, int height, GLint xoffset, GLint yoffset, GLint swidth, GLint sheight, GLuint *plan);
//-------------------------------------------------------------------------------
//image        : image handle id
//data        : for updated image data pointer
//xoffset    : the x dim start point reference to pixel (0,0)
//xoffset    : the y dim start point reference to pixel (0,0)
//width        : width to be updated
//height    : height to be updated
IG_API GLvoid igUpdateSubImage( GLuint image, GLbyte * data, GLint xoffset, GLint yoffset, GLuint width, GLuint height );
//-------------------------------------------------------------------------------
//target_width    : target image width
//target_height : target image height
//vpg_width        : virtual processing group width
//vpg_height    : virtual processing group height
//buffers        : The buffer pointer indicates both VPE and VPG handle id
//Return        : the VPE element number for processing
IG_API GLuint igCreateVPE( int target_width, int target_height, int vpg_width, int vpg_height, GLuint *buffers );
//-------------------------------------------------------------------------------
//target_width    : target image width
//target_height : target image height
//vpg_width        : virtual processing group width
//vpg_height    : virtual processing group height
//buffers        : The buffer pointer indicates both VPE and VPG handle id
//ref_buffer    : The VPG reference data buffer pointer
//Return        : the VPE element number for processing
IG_API GLuint igCreateVPE( int target_width, int target_height, int vpg_width, int vpg_height, GLuint *buffers, GLfloat * ref_buffer);
//-------------------------------------------------------------------------------
//target_width    : target image width
//target_height : target image height
//roi_x            : x start of region of interest
//roi_y            : y start of region of interest
//roi_width        : width of region of interest
//roi_height    : height of region of interest
//buffers        : The buffer pointer indicates both VPE and VPG handle id
//Return        : the VPE element number for processing
IG_API GLuint igCreateVPE( int target_width, int target_height, int roi_x, int roi_y, int roi_width, int roi_height, GLuint *buffers);
//-------------------------------------------------------------------------------
//target_width    : target image width
//target_height : target image height
//roi_x            : x start coordinate of region of interest
//roi_y            : y start coordinate of region of interest
//roi_width        : width of region of interest
//roi_height    : height of region of interest
//buffers        : The buffer pointer indicates both VPE and VPG handle id
//ref_buffer    : The VPG reference data buffer pointer
//Return        : the VPE element number for processing
IG_API GLuint igCreateVPE( int target_width, int target_height, int roi_x, int roi_y, int roi_width, int roi_height, GLuint *buffers, GLfloat * ref_buffer);
//-------------------------------------------------------------------------------
//target_width    : target image width
//target_height : target image height
//elenum        : the VPE element number for processing
GLvoid igProc(int target_width, int target_height, GLuint elenum);
//-------------------------------------------------------------------------------
//target_width    : target image width
//target_height : target image height
//vpg_width        : virtual processing group width
//vpg_height    : virtual processing group height
//vpg_x            : virtual processing group x coordinate
//vpg_y            : virtual processing group y coordinate
IG_API GLvoid igProc(int target_width, int target_height, int vpg_width, int vpg_height, int vpg_x, int vpg_y);
//-------------------------------------------------------------------------------
//target_width    : target image width
//target_height : target image height
//vpg_width        : virtual processing group width
//vpg_height    : virtual processing group height
//vpg_x            : virtual processing group x coordinate
//vpg_y            : virtual processing group y coordinate
//new_vpg_x        : new virtual processing group x coordinate
//new_vpg_y        : new virtual processing group y coordinate
IG_API GLvoid igProc(int target_width, int target_height, int vpg_width, int vpg_height, int vpg_x, int vpg_y, int new_vpg_x, int new_vpg_y);
//-------------------------------------------------------------------------------
//target_width    : target image width
//target_height : target image height
//roi_x            : x start coordinate of region of interest
//roi_y            : y start coordinate of region of interest
IG_API GLvoid igProc(int target_width, int target_height, int roi_x, int roi_y);
//-------------------------------------------------------------------------------
//program        : the program handle id
//img_name        : the image name in sahder
//tex_unit        : the texture unit to bind, GL_TEXTURE0, GL_TEXTURE1, GL_TEXTURE2, GL_TEXTURE3,...
//texture        : the image handle id
IG_API GLvoid igProgramBindImage(GLuint program, const char * img_name, GLuint tex_unit, GLuint image);
//-------------------------------------------------------------------------------
//shader        : the shader handle id
//IG_API GLvoid igFreeShader(GLuint shader);
//-------------------------------------------------------------------------------
//program        : the program handle id
//IG_API GLvoid igFreeProgram(GLuint program);
//-------------------------------------------------------------------------------
//n                : number of the texture handle to be free
//buffers        : image handle id pointer
//IG_API GLvoid igFreeImage(GLuint n, const GLuint* image);
//-------------------------------------------------------------------------------
//buffers        : The buffer pointer indicates both VPE and VPG handle id
//IG_API GLvoid igFreeVPE(const GLuint* buffers);
//-------------------------------------------------------------------------------
//target_width    : target image width
//target_height : target image height
//target_ptr    : The pointer for target data storage
//format        : GL_RGBA, GL_LUMINANCE
IG_API GLvoid igReadTarget(GLuint target_width, GLuint target_height, void* target_ptr, GLenum format);
//-------------------------------------------------------------------------------
//shader    : shader handle
//Return    : the status if the shader bad been compiled successfully; ture: success, false: Fail
IG_API bool    igCheckCompile(GLuint shader);
//-------------------------------------------------------------------------------
//program    : program handle
//Return    : the status if the program bad been linked successfully; ture: success, false: Fail
IG_API bool    igCheckLink(GLuint program);
//-------------------------------------------------------------------------------
//program        : the program handle id
//tex_unit        : the texture unit to bind, GL_TEXTURE0, GL_TEXTURE1, GL_TEXTURE2, GL_TEXTURE3,...
//image            : the image handle id
//param            : the image parameter for min filter, GL_NEAREST, GL_LINEAR
//param            : the image parameter for mag filter, GL_NEAREST, GL_LINEAR
IG_API GLvoid igImageParameter(GLuint image, GLuint tex_unit, GLint min_param, GLint mag_param);
//-------------------------------------------------------------------------------
//target_width    : target image width
//target_height : target image height
//fbo            : return the frame buffer object id
//rbo            : return the render buffer object id
//IG_API bool igCreateBufferObject(int target_width, int target_height, GLuint &fbo, GLuint &rbo);
//-------------------------------------------------------------------------------
//target_width    : target image width
//target_height : target image height
//fbo            : return the frame buffer object id
//texture        : for render to texture
//format        : texture format
IG_API bool igCreateBufferObject(int target_width, int target_height, GLuint &fbo, GLuint &texture, GLenum format);
//-------------------------------------------------------------------------------
//fbo            : input the frame buffer resource id
//rbo            : input the render buffer resource id
//IG_API GLvoid igFreeBufferObject(GLuint &fbo, GLuint &rbo);
//-------------------------------------------------------------------------------
//fbo            : return the frame buffer id
IG_API GLvoid igFreeBufferObject(GLuint &fbo);
//-------------------------------------------------------------------------------
//vs        : vertex shader handle
//fs        : fragment shader handle
//program    : current program handle
//Return    : the program handle id
IG_API GLvoid igUpdateShader(GLuint cur_shader, GLuint new_shader, GLuint program);
//-------------------------------------------------------------------------------
//filename    : for 32-bit floating image file
//mode        : GL_REPEAT, GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT
//width        : image's width
//height    : image's height
//Return    : the image handle id
GLuint igLoadFloatingImage( const char * filename, GLint mode, int width, int height );
GLuint igLoadFloatingImage( float* data, GLint mode, int width, int height, int version );

#endif
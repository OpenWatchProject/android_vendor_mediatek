/**
 * \brief The imaGPU library header
 * \details imaGPU is a GLES2.0 base framework for image processing
 * \author Eddie Tsao
 * \version 1.0
 */
#ifndef _UTIL_IMA_GPU_H_
#define _UTIL_IMA_GPU_H_

#include "MTKUtilCommon.h"
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#define VERTEX_ARRAY    0           ///< vertex array index
#define TEXTURE_ARRAY   1           ///< texture array index
#define REF_ARRAY       2           ///< reference array index

#define IG_YUV422P      0x8000      ///< ImaGPU YUV422 planar format
#define IG_YUV420P      0x8001      ///< ImaGPU YUV420 planar format
#define IG_YUV420SP     0x8002      ///< ImaGPU YUV420 special planar format
#define IG_RGB          0x8003      ///< ImaGPU RGB format

/// vertex format
typedef struct
{
    GLfloat x;
    GLfloat y;
    GLfloat u;
    GLfloat v;
} vtx_fmt;

/**
 *  \details load shader string
 *  \fn UTIL_ERRCODE_ENUM igLoadShaderString(GLuint *shd_handle, const char *filestr, GLenum type)
 *  \param[out] shd_handle shader handle
 *  \param[in] filestr shader string
 *  \param[in] type #GL_VERTEX_SHADER or #GL_FRAGMENT_SHADER
 *  \return utility error code
 */
UTIL_ERRCODE_ENUM igLoadShaderString(GLuint *shd_handle, const char *filestr, GLenum type);

/**
 *  \details load shader binary
 *  \fn UTIL_ERRCODE_ENUM igLoadShaderBinary(GLuint *shd_handle, const char *shd_bin, GLint shd_size, GLenum type)
 *  \param[out] shd_handle shader handle
 *  \param[in] shd_bin shader binary
 *  \param[in] shd_size size of shader binary
 *  \param[in] type #GL_VERTEX_SHADER or #GL_FRAGMENT_SHADER
 *  \return utility error code
 */
UTIL_ERRCODE_ENUM igLoadShaderBinary(GLuint *shd_handle, const char *shd_bin, GLint shd_size, GLenum type);

/**
 *  \details create program object handle
 *  \fn UTIL_ERRCODE_ENUM igCreateProgram(GLuint *po, GLuint vs, GLuint fs, GLfloat *ref_buffer)
 *  \param[out] po program object handle
 *  \param[in] vs vertex shader handle
 *  \param[in] fs fragment shader handle
 *  \param[in] ref_buffer reference buffer for vpg_ref. Set 0 if there is no ref_buffer
 *  \return utility error code
 */
UTIL_ERRCODE_ENUM igCreateProgram(GLuint *po, GLuint vs, GLuint fs, GLfloat *ref_buffer);

/**
 *  \details load single plane image
 *  \fn UTIL_ERRCODE_ENUM igLoadImage(void *data, GLint mode, GLenum format, int width, int height)
 *  \param[in] data for image data pointer
 *  \param[in] mode #GL_REPEAT, #GL_CLAMP_TO_EDGE or #GL_MIRRORED_REPEAT
 *  \param[in] format #IG_YUV422P, #IG_YUV420P, #IG_YUV420SP or #IG_RGB
 *  \param[in] width image's width
 *  \param[in] height image's height
 *  \return utility error code
 */
UTIL_ERRCODE_ENUM igLoadImage(void *data, GLint mode, GLenum format, int width, int height);

/**
 *  \details load multiple plane image
 *  \fn UTIL_ERRCODE_ENUM igLoadImage(void * data, GLint mode, GLenum format, int width, int height, GLuint *plan)
 *  \param[in] data for image data pointer
 *  \param[in] mode #GL_REPEAT, #GL_CLAMP_TO_EDGE or #GL_MIRRORED_REPEAT
 *  \param[in] format #IG_YUV422P, #IG_YUV420P, #IG_YUV420SP or #IG_RGB
 *  \param[in] width image's width
 *  \param[in] height image's height
 *  \param[in] plan texture handle
 *  \return utility error code
 */
UTIL_ERRCODE_ENUM igLoadImage(void * data, GLint mode, GLenum format, int width, int height, GLuint *plan);

/**
 *  \details create VPE
 *  \fn UTIL_ERRCODE_ENUM igCreateVPE(GLint *elenum, GLint *vpg_size, vtx_fmt *vtx_data, GLushort *ele_data, GLuint *buffers)
 *  \param[in] elenum the VPE element number for processing
 *  \param[in] vpg_size (vpg_width, vpg_height)
 *  \param[in] vtx_data vertex data array
 *  \param[in] ele_data element data array
 *  \param[in] buffers The buffer pointer indicates both VPE and VPG handle id
 *  \return utility error code
 */
UTIL_ERRCODE_ENUM igCreateVPE(GLint *elenum, GLint *vpg_size, vtx_fmt *vtx_data, GLushort *ele_data, GLuint *buffers);

/**
 *  \details free shader handle
 *  \fn UTIL_ERRCODE_ENUM igFreeShader(GLuint shader)
 *  \param[in] shader shader handle id
 *  \return utility error code
 */
UTIL_ERRCODE_ENUM igFreeShader(GLuint shader);

/**
 *  \details free program handle
 *  \fn UTIL_ERRCODE_ENUM igFreeProgram(GLuint program)
 *  \param[in] program the program handle id
 *  \return utility error code
 */
UTIL_ERRCODE_ENUM igFreeProgram(GLuint program);

/**
 *  \details free texture handle
 *  \fn UTIL_ERRCODE_ENUM igFreeImage(GLuint n, const GLuint* image)
 *  \param[in] n number of the texture handle to be free
 *  \param[in] image image handle id pointer
 *  \return utility error code
 */
UTIL_ERRCODE_ENUM igFreeImage(GLuint n, const GLuint* image);

/**
 *  \details free VPE
 *  \fn UTIL_ERRCODE_ENUM igFreeVPE(const GLuint* buffers)
 *  \param[in] buffers The buffer pointer indicates both VPE and VPG handle id
 *  \return utility error code
 */
UTIL_ERRCODE_ENUM igFreeVPE(const GLuint* buffers);

/**
 *  \details create buffer object
 *  \fn UTIL_ERRCODE_ENUM igCreateBufferObject(int target_width, int target_height, GLuint &fbo, GLuint &rbo)
 *  \param[in] target_width target image width
 *  \param[in] target_height target image height
 *  \param[out] fbo return the frame buffer object id
 *  \param[out] rbo return the render buffer object id
 *  \return utility error code
 */
UTIL_ERRCODE_ENUM igCreateBufferObject(int target_width, int target_height, GLuint &fbo, GLuint &rbo);

/**
 *  \details free buffer object
 *  \fn UTIL_ERRCODE_ENUM igFreeBufferObject(GLuint &fbo, GLuint &rbo)
 *  \param[in] fbo input the frame buffer resource id
 *  \param[in] rbo input the render buffer resource id
 *  \return utility error code
 */
UTIL_ERRCODE_ENUM igFreeBufferObject(GLuint &fbo, GLuint &rbo);

/**
 *  \details start rendering
 *  \fn igStartRendering(GLuint program, const char *img_name, GLuint image, GLuint elenum, P_UTIL_CLIP_IMAGE_STRUCT src)
 *  \param[in] program the program handle id
 *  \param[in] img_name image name in shader code
 *  \param[in] image texture handle id
 *  \param[in] elenum the VPE element number for processing
 *  \param[in] src data structure of processing image
 *  \return utility error code
 */
UTIL_ERRCODE_ENUM igStartRendering(GLuint program, const char *img_name, GLuint image, GLuint elenum, P_UTIL_CLIP_IMAGE_STRUCT src);

#endif /* _UTIL_IMA_GPU_H_ */


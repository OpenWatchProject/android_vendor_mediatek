//-------------------------------------------------------------------------------
//-- Title        : The imaGPU library implementation
//
//-- Creator    : Eddie Tsao
//
//-- Version    : v0
//
//-- Description : imaGPU is a GLES2.0 base framework for image processing
//
//-- Add glPixelStorei(GL_UNPACK_ALIGNMENT, 1) for igLoadImage.
//-------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "imagpu.h"

//-------------------------------------------------------------------------------
//filename    : for shader file
//type        : GL_VERTEX_SHADER, GL_FRAGMENT_SHADER
//Return    : the shader handle id
//GLuint igLoadShader( const char * filename, GLenum type )
GLuint igLoadShader( const char * pData, GLenum type )
{
    //char* pData;
    //FILE* pFile;
//    FILE* pFile = fopen_s(filename, "rb");
    /*
    pFile = fopen(filename, "rb");
    if (pFile)
    {
        // Get the file size
        fseek(pFile, 0, SEEK_END);
        int size = ftell(pFile);
        fseek(pFile, 0, SEEK_SET);

        // read the data, append a 0 byte as the data might represent a string
        pData = new char[size + 1];
        pData[size] = '\0';
        size_t BytesRead = fread(pData, 1, size, pFile);

        if (BytesRead != size)
        {
            delete [] pData;
            return 0;
        }
        fclose(pFile);
    }
    else
    {
        return 0;
    }
*/
    // Create the shader object
    GLuint Shader;
    Shader = glCreateShader(type);

    // Load the source
    glShaderSource(Shader, 1, (const char**)&pData, NULL);

    // Compile the source code
    glCompileShader(Shader);

    return Shader;
}

//-------------------------------------------------------------------------------
//shader    : shader handle
//Return    : the status if the shader bad been compiled successfully; ture: success, false: Fail
bool igCheckCompile(GLuint shader)
{
    GLint bShaderCompiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &bShaderCompiled);
    if ((glGetError() != GL_NO_ERROR) || !bShaderCompiled)
    {
        return 0;
    }
    return 1;
}

//-------------------------------------------------------------------------------
//vs        : vertex shader handle
//fs        : fragment shader handle
//ref_buffer: reference buffer for vpg_ref
//Return    : the program handle id
GLuint igCreateProgram(GLuint vs, GLuint fs, GLfloat * ref_buffer )
{
    GLuint ProgramObject = glCreateProgram();

    // Attach the fragment and vertex shaders to it
    glAttachShader(ProgramObject, vs);
    glAttachShader(ProgramObject, fs);

    // Here needs to be implement the imaGPU shader framework
    // define VPE_TC
    // define VPE_SC
    // define VPE_SD
    // define VPG_REF

    // Bind the custom vertex attribute "myVertex" to location VERTEX_ARRAY
    glBindAttribLocation(ProgramObject, VERTEX_ARRAY, "_position");//"vpg_tc"
    glBindAttribLocation(ProgramObject, TEXTURE_ARRAY,"_texcoord");//"vpg_sc"
    //if (ref_buffer)
    //    glBindAttribLocation(ProgramObject, REF_ARRAY, "vpg_in_ref");

    // Link the program
    glLinkProgram(ProgramObject);

    glUseProgram(ProgramObject);
    ref_buffer = NULL;
    return ProgramObject;
}

//-------------------------------------------------------------------------------
//program    : program handle
//Return    : the status if the program bad been linked successfully; ture: success, false: Fail
bool    igCheckLink(GLuint program)
{
    GLint bLinked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &bLinked);
    if ((glGetError() != GL_NO_ERROR) || !bLinked)
    {
        return 0;
    }
    return 1;
}

//-------------------------------------------------------------------------------
//filename    : for ppm image file
//mode        : GL_REPEAT, GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT
//Return    : the image handle id
/*
GLuint igLoadImage( const char * filename, GLint mode )
{
    GLuint texture;
    unsigned char * data;
    FILE * file;

    int width = 0xf0f0, height = 0xf0f0;

    // open texture data
    file = fopen(filename, "rb" );
    if ( file == NULL ) return 0;

    //Parsing the header
    bool bParsing = true;
    char ch, ch_buf[10];
    int i,j,k;
    i=0;
    while (bParsing)
    {
        ch = fgetc( file );
        if (ch==0x0A || ch==0x20)    //"\n" and " "
        {
            ch_buf[i] = '\0';
            i = 0;
            if (ch_buf[0] != 0x50)    //"P"
            {
                if (width==0xf0f0)
                {
                    width = atoi(ch_buf);
                }
                else if (height==0xf0f0)
                {
                    height = atoi(ch_buf);
                }
                else
                    bParsing = false;
            }
        }
        else
        {
            ch_buf[i] = ch;
            i++;
        }

    }
    // allocate buffer
    data = (unsigned char *)malloc( width * height * 3 );

    // read texture data
    for (j=height-1; j>=0; j--)
    {
        for (i=0; i<width; i++)
        {
            for (k=0; k<3; k++)
            {
                data[width*3*j + 3*i + k] = fgetc( file );
            }
        }
    }
    fclose( file );

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);    //09,05,06 Eddie

    // allocate a texture name
    glGenTextures( 1, &texture );

    // select our current texture
    glBindTexture( GL_TEXTURE_2D, texture );

    // when texture area is small, bilinear filter the closest MIP map
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    // when texture area is large, bilinear filter the first MIP map
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    // if wrap is true, the texture wraps over at the edges (repeat)
    //       ... false, the texture ends at the edges (clamp)
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode );

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    // free buffer
    free( data );

    return texture;
}
*/
//-------------------------------------------------------------------------------
//filename    : for RGB raw image file
//mode        : GL_REPEAT, GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT
//width        : image's width
//height    : image's height
//Return    : the image handle id
/*
GLuint igLoadImage( const char * filename, GLint mode , int width, int height )
{
    GLuint texture;
    unsigned char * data;
    FILE * file;

    // open texture data
    file = fopen(filename, "rb" );
    if ( file == NULL ) return 0;

    // allocate buffer
    data = (unsigned char *)malloc( width * height * 3 );

    // read texture data
    fread( data, width * height * 3, 1, file );
    fclose( file );

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);    //09,05,06 Eddie

    // allocate a texture name
    glGenTextures( 1, &texture );

    // select our current texture
    glBindTexture( GL_TEXTURE_2D, texture );

    // when texture area is small, bilinear filter the closest MIP map
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    // when texture area is large, bilinear filter the first MIP map
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    // if wrap is true, the texture wraps over at the edges (repeat)
    //       ... false, the texture ends at the edges (clamp)
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    // free buffer
    free( data );

    return texture;
}
*/
//-------------------------------------------------------------------------------
//filename    : for 32-bit floating image file
//mode        : GL_REPEAT, GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT
//width        : image's width
//height    : image's height
//Return    : the image handle id
GLuint igLoadFloatingImage( const char * filename, GLint mode, int width, int height )
{
    GLuint texture;
    //unsigned char * data;
    float* data;
    FILE * file;

    // open texture data
    file = fopen(filename, "rb" );
    if ( file == NULL ) return 0;

    // allocate buffer
    data = (float *)malloc( width * height * sizeof(float) );

    // read texture data
    fread( data, width * height * sizeof(float), 1, file );
    fclose( file );

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);    //09,05,06 Eddie

    // allocate a texture name
    glGenTextures( 1, &texture );

    // select our current texture
    glBindTexture(GL_TEXTURE_2D , texture );//GL_OES_texture_float

    // when texture area is small, bilinear filter the closest MIP map
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//GL_NEAREST
    // when texture area is large, bilinear filter the first MIP map
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // if wrap is true, the texture wraps over at the edges (repeat)
    //       ... false, the texture ends at the edges (clamp)
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_FLOAT, data);//GL_OES_texture_float
    // free buffer
    free( data );

    return texture;
}
GLuint igLoadFloatingImage( float* data, GLint mode, int width, int height, int version )
{
    GLuint texture;
    //unsigned char * data;
    //float* data;

    // allocate buffer
    //data = (float *)malloc( width * height * sizeof(float) );



    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);    //09,05,06 Eddie

    // allocate a texture name
    glGenTextures( 1, &texture );

    // select our current texture
    glBindTexture(GL_TEXTURE_2D , texture );//GL_OES_texture_float

    // when texture area is small, bilinear filter the closest MIP map
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // when texture area is large, bilinear filter the first MIP map
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

    // if wrap is true, the texture wraps over at the edges (repeat)
    //       ... false, the texture ends at the edges (clamp)
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode);
    if(version == 2)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_FLOAT, data);//GL_OES_texture_float
    else if(version == 3)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, data);
    // free buffer
    //free( data );

    return texture;
}

//-------------------------------------------------------------------------------
//image        : image handle id
//data        : for updated image data pointer
//xoffset    : the x dim start point reference to pixel (0,0)
//xoffset    : the y dim start point reference to pixel (0,0)
//width        : width to be updated
//height    : height to be updated
GLvoid igUpdateSubImage( GLuint image, GLbyte * data, GLint xoffset, GLint yoffset, GLuint width, GLuint height )
{
    // select our current texture
    glBindTexture( GL_TEXTURE_2D, image );

    // when texture area is small, bilinear filter the closest MIP map
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    // when texture area is large, bilinear filter the first MIP map
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    glTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);

}

//-------------------------------------------------------------------------------
//target_width    : target image width
//target_height : target image height
//vpg_width        : virtual processing group width
//vpg_height    : virtual processing group height
//buffers        : The buffer pointer indicates both VPE and VPG handle id
//Return        : the VPE element number for processing
GLuint igCreateVPE( int target_width, int target_height, int vpg_width, int vpg_height, GLuint *buffers )
{
    int i,j, index, start;

    if ((vpg_width <= 0) || (vpg_height <= 0))
        return 0;

    target_width = 0;
    target_height = 0;
    
    GLfloat gridx = 2/GLfloat(vpg_width);
    GLfloat gridy = 2/GLfloat(vpg_height);
    GLfloat gridu = 1/GLfloat(vpg_width);
    GLfloat gridv = 1/GLfloat(vpg_height);

    /* Generate array and element buffers. */
    glGenBuffers(2, buffers);
    /* Bind the array and element buffers. */
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
    struct vtx_fmt
    {
        GLfloat x;
        GLfloat y;
        GLfloat u;
        GLfloat v;
    };
    vtx_fmt * vtx_data = new vtx_fmt[(vpg_width+1)*2*vpg_height];

    if (vtx_data)
    {
        //Prepare vtx data
        index = 2;
        vtx_data[0].x = -1.0;
        vtx_data[0].y = -1.0;
        vtx_data[0].u = 0.0;
        vtx_data[0].v = 0.0;
        vtx_data[1].x = -1.0;
        vtx_data[1].y = vtx_data[0].y + gridy;
        vtx_data[1].u = 0.0;
        vtx_data[1].v = vtx_data[0].v + gridv;
        for (i=0; i<vpg_height; i++)
        {
            for (j=0; j<vpg_width; j++)
            {
                vtx_data[index].x = vtx_data[index-2].x + gridx;
                vtx_data[index].y = vtx_data[index-2].y;
                vtx_data[index].u = vtx_data[index-2].u + gridu;
                vtx_data[index].v = vtx_data[index-2].v;
                index ++;
                vtx_data[index].x = vtx_data[index-2].x + gridx;
                vtx_data[index].y = vtx_data[index-2].y;
                vtx_data[index].u = vtx_data[index-2].u + gridu;
                vtx_data[index].v = vtx_data[index-2].v;
                index ++;
            }
            if (i+1 != vpg_height)
            {
                vtx_data[index].x =  -1.0;
                vtx_data[index].y = vtx_data[index-1].y;
                vtx_data[index].u = 0.0;
                vtx_data[index].v = vtx_data[index-1].v;
                index ++;
                vtx_data[index].x =  -1.0;
                vtx_data[index].y = vtx_data[index-1].y + gridy;
                vtx_data[index].u = 0.0;
                vtx_data[index].v = vtx_data[index-1].v + gridv;
                index ++;
            }
        }
        /* Upload the array buffer. */
        glBufferData(GL_ARRAY_BUFFER, sizeof(vtx_fmt)*(vpg_width+1)*2*vpg_height, vtx_data, GL_STATIC_DRAW);
    }

    /* Free the array buffer */
    delete []vtx_data;

    //Prepare element data
    GLuint * ele_data = new GLuint[6*vpg_width*vpg_height];

    if (ele_data)
    {
        index = 0;
        start = 0;
        for (i=0; i<vpg_height; i++)
        {
            for (j=0; j<vpg_width; j++)
            {
                ele_data[index] = start;
                ele_data[index+1] = start + 1;
                ele_data[index+2] = start + 2;

                ele_data[index+3] = ele_data[index+2];
                ele_data[index+4] = ele_data[index+1];
                ele_data[index+5] = start + 3;
                index += 6;
                start += 2;
            }
            start += 2;
        }
        /* Upload the array buffer. */
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*6*vpg_width*vpg_height, ele_data, GL_STATIC_DRAW);
    }

    /* Free the array buffer */
    delete []ele_data;

    /* Enable vertex array attribute 0 (position). */
    glEnableVertexAttribArray(VERTEX_ARRAY);
    glVertexAttribPointer(VERTEX_ARRAY, 2, GL_FLOAT, GL_FALSE, sizeof(vtx_fmt), 0);

    /* Enable vertex array attribute 1 (texture coordiate). */
    glEnableVertexAttribArray(TEXTURE_ARRAY);
    glVertexAttribPointer(TEXTURE_ARRAY, 2, GL_FLOAT, GL_FALSE, sizeof(vtx_fmt), (GLvoid *)(sizeof(GLfloat)*2));

    return 6*vpg_width*vpg_height;
}

//-------------------------------------------------------------------------------
//target_width    : target image width
//target_height : target image height
//vpg_width        : virtual processing group width
//vpg_height    : virtual processing group height
//buffers        : The buffer pointer indicates both VPE and VPG handle id
//ref_buffer    : The VPG reference data buffer pointer
//Return        : the VPE element number for processing
GLuint igCreateVPE( int target_width, int target_height, int vpg_width, int vpg_height, GLuint *buffers, GLfloat * ref_buffer)
{
    int i,j, index, start;
    
    target_width = 0;
    target_height = 0;
    
    GLfloat gridx = 2/GLfloat(vpg_width);
    GLfloat gridy = 2/GLfloat(vpg_height);
    GLfloat gridu = 1/GLfloat(vpg_width);
    GLfloat gridv = 1/GLfloat(vpg_height);

    /* Generate array and element buffers. */
    glGenBuffers(2, buffers);
    /* Bind the array and element buffers. */
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
    struct vtx_fmt
    {
        GLfloat x;
        GLfloat y;
        GLfloat u;
        GLfloat v;
        GLfloat ref[4];
    };
    vtx_fmt * vtx_data = new vtx_fmt[vpg_width*4*vpg_height];

    //Prepare vtx data
    index = 0;
    for (i=0; i<vpg_height; i++)
    {
        for (j=0; j<vpg_width; j++)
        {
            start = index/4;
            vtx_data[index].x = -1 + gridx*j;
            vtx_data[index].y = -1 + gridy*i;
            vtx_data[index].u = gridu*j;
            vtx_data[index].v = gridv*i;
            vtx_data[index].ref[0] = ref_buffer[16*start + 0];
            vtx_data[index].ref[1] = ref_buffer[16*start + 1];
            vtx_data[index].ref[2] = ref_buffer[16*start + 2];
            vtx_data[index].ref[3] = ref_buffer[16*start + 3];
            vtx_data[index+1].x = vtx_data[index].x;
            vtx_data[index+1].y = vtx_data[index].y + gridy;
            vtx_data[index+1].u = vtx_data[index].u;
            vtx_data[index+1].v = vtx_data[index].v + gridv;
            vtx_data[index+1].ref[0] = ref_buffer[16*start + 4];
            vtx_data[index+1].ref[1] = ref_buffer[16*start + 5];
            vtx_data[index+1].ref[2] = ref_buffer[16*start + 6];
            vtx_data[index+1].ref[3] = ref_buffer[16*start + 7];
            vtx_data[index+2].x = vtx_data[index].x + gridx;
            vtx_data[index+2].y = vtx_data[index].y;
            vtx_data[index+2].u = vtx_data[index].u + gridu;
            vtx_data[index+2].v = vtx_data[index].v;
            vtx_data[index+2].ref[0] = ref_buffer[16*start + 8];
            vtx_data[index+2].ref[1] = ref_buffer[16*start + 9];
            vtx_data[index+2].ref[2] = ref_buffer[16*start + 10];
            vtx_data[index+2].ref[3] = ref_buffer[16*start + 11];
            vtx_data[index+3].x = vtx_data[index+1].x + gridx;
            vtx_data[index+3].y = vtx_data[index+1].y;
            vtx_data[index+3].u = vtx_data[index+1].u + gridu;
            vtx_data[index+3].v = vtx_data[index+1].v;
            vtx_data[index+3].ref[0] = ref_buffer[16*start + 12];
            vtx_data[index+3].ref[1] = ref_buffer[16*start + 13];
            vtx_data[index+3].ref[2] = ref_buffer[16*start + 14];
            vtx_data[index+3].ref[3] = ref_buffer[16*start + 15];
            index += 4;
        }
    }
    /* Upload the array buffer. */
    glBufferData(GL_ARRAY_BUFFER, sizeof(vtx_fmt)*vpg_width*4*vpg_height, vtx_data, GL_STATIC_DRAW);

    /* Free the array buffer */
    delete []vtx_data;

    //Prepare element data
    GLushort * ele_data = new GLushort[6*vpg_width*vpg_height];
    index = 0;
    start = 0;
    for (i=0; i<vpg_height; i++)
    {
        for (j=0; j<vpg_width; j++)
        {
            ele_data[index] = start;
            ele_data[index+1] = start + 1;
            ele_data[index+2] = start + 2;

            ele_data[index+3] = ele_data[index+2];
            ele_data[index+4] = ele_data[index+1];
            ele_data[index+5] = start + 3;
            index += 6;
            start += 4;
        }
    }
    /* Upload the array buffer. */
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)*6*vpg_width*vpg_height, ele_data, GL_STATIC_DRAW);

    /* Free the array buffer */
    delete []ele_data;

    /* Enable vertex array attribute 0 (position). */
    glEnableVertexAttribArray(VERTEX_ARRAY);
    glVertexAttribPointer(VERTEX_ARRAY, 2, GL_FLOAT, GL_FALSE, sizeof(vtx_fmt), 0);

    /* Enable vertex array attribute 1 (texture coordiate). */
    glEnableVertexAttribArray(TEXTURE_ARRAY);
    glVertexAttribPointer(TEXTURE_ARRAY, 2, GL_FLOAT, GL_FALSE, sizeof(vtx_fmt), (GLvoid *)(sizeof(GLfloat)*2));

    /* Enable vertex array attribute 1 (vpg reference). */
    glEnableVertexAttribArray(REF_ARRAY);
    glVertexAttribPointer(REF_ARRAY, 4, GL_FLOAT, GL_FALSE, sizeof(vtx_fmt), (GLvoid *)(sizeof(GLfloat)*4));

    return 6*vpg_width*vpg_height;
}

//-------------------------------------------------------------------------------
//target_width    : target image width
//target_height : target image height
//roi_x            : x start of region of interest
//roi_y            : y start of region of interest
//roi_width        : width of region of interest
//roi_height    : height of region of interest
//buffers        : The buffer pointer indicates both VPE and VPG handle id
//Return        : the VPE element number for processing
GLuint igCreateVPE( int target_width, int target_height, int roi_x, int roi_y, int roi_width, int roi_height, GLuint *buffers)
{
    GLfloat gridx = 2/GLfloat(target_width);
    GLfloat gridy = 2/GLfloat(target_height);
    GLfloat gridu = 1/GLfloat(target_width);
    GLfloat gridv = 1/GLfloat(target_height);

    /* Generate array and element buffers. */
    glGenBuffers(2, buffers);
    /* Bind the array and element buffers. */
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
    struct vtx_fmt
    {
        GLfloat x;
        GLfloat y;
        GLfloat u;
        GLfloat v;
    };
    vtx_fmt * vtx_data = new vtx_fmt[4];
    //Prepare vtx data
    vtx_data[0].x = -1 + roi_x * gridx;
    vtx_data[0].y = -1 + roi_y * gridy;
    vtx_data[0].u = roi_x * gridu;
    vtx_data[0].v = roi_y * gridv;

    vtx_data[1].x = vtx_data[0].x;
    vtx_data[1].y = -1 + (roi_y+roi_height) * gridy;
    vtx_data[1].u = vtx_data[0].u;
    vtx_data[1].v = (roi_y+roi_height) * gridv;

    vtx_data[2].x = -1 + (roi_x+roi_width) * gridx;
    vtx_data[2].y = vtx_data[0].y;
    vtx_data[2].u = (roi_x+roi_width) * gridu;
    vtx_data[2].v = vtx_data[0].v;

    vtx_data[3].x = vtx_data[2].x;
    vtx_data[3].y = vtx_data[1].y;
    vtx_data[3].u = vtx_data[2].u;
    vtx_data[3].v = vtx_data[1].v;

    /* Upload the array buffer. */
    glBufferData(GL_ARRAY_BUFFER, sizeof(vtx_fmt)*4, vtx_data, GL_STATIC_DRAW);

    /* Free the array buffer */
    delete []vtx_data;

    //Prepare element data
    GLushort * ele_data = new GLushort[6];
    ele_data[0] = 0;
    ele_data[1] = 1;
    ele_data[2] = 2;
    ele_data[3] = 2;
    ele_data[4] = 1;
    ele_data[5] = 3;
    /* Upload the array buffer. */
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)*6, ele_data, GL_STATIC_DRAW);

    /* Free the array buffer */
    delete []ele_data;

    /* Enable vertex array attribute 0 (position). */
    glEnableVertexAttribArray(VERTEX_ARRAY);
    glVertexAttribPointer(VERTEX_ARRAY, 2, GL_FLOAT, GL_FALSE, sizeof(vtx_fmt), 0);

    /* Enable vertex array attribute 1 (texture coordiate). */
    glEnableVertexAttribArray(TEXTURE_ARRAY);
    glVertexAttribPointer(TEXTURE_ARRAY, 2, GL_FLOAT, GL_FALSE, sizeof(vtx_fmt), (GLvoid *)(sizeof(GLfloat)*2));

    return 6;
}

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
GLuint igCreateVPE( int target_width, int target_height, int roi_x, int roi_y, int roi_width, int roi_height, GLuint *buffers, GLfloat * ref_buffer)
{
    GLfloat gridx = 2/GLfloat(target_width);
    GLfloat gridy = 2/GLfloat(target_height);
    GLfloat gridu = 1/GLfloat(target_width);
    GLfloat gridv = 1/GLfloat(target_height);

    if (!glIsBuffer(buffers[0]))
    {
        /* Generate array buffers. */
        glGenBuffers(1, &buffers[0]);
        glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    }
    if (!glIsBuffer(buffers[1]))
    {
        /* Generate element buffers. */
        glGenBuffers(1, &buffers[1]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
    }

    struct vtx_fmt
    {
        GLfloat x;
        GLfloat y;
        GLfloat u;
        GLfloat v;
        GLfloat ref[4];
    };
    vtx_fmt * vtx_data = new vtx_fmt[4];
    //Prepare vtx data
    vtx_data[0].x = -1 + roi_x * gridx;
    vtx_data[0].y = -1 + roi_y * gridy;
    vtx_data[0].u = roi_x * gridu;
    vtx_data[0].v = roi_y * gridv;
    vtx_data[0].ref[0] = ref_buffer[0];
    vtx_data[0].ref[1] = ref_buffer[1];
    vtx_data[0].ref[2] = ref_buffer[2];
    vtx_data[0].ref[3] = ref_buffer[3];

    vtx_data[1].x = vtx_data[0].x;
    vtx_data[1].y = -1 + (roi_y+roi_height) * gridy;
    vtx_data[1].u = vtx_data[0].u;
    vtx_data[1].v = (roi_y+roi_height) * gridv;
    vtx_data[1].ref[0] = ref_buffer[4];
    vtx_data[1].ref[1] = ref_buffer[5];
    vtx_data[1].ref[2] = ref_buffer[6];
    vtx_data[1].ref[3] = ref_buffer[7];

    vtx_data[2].x = -1 + (roi_x+roi_width) * gridx;
    vtx_data[2].y = vtx_data[0].y;
    vtx_data[2].u = (roi_x+roi_width) * gridu;
    vtx_data[2].v = vtx_data[0].v;
    vtx_data[2].ref[0] = ref_buffer[8];
    vtx_data[2].ref[1] = ref_buffer[9];
    vtx_data[2].ref[2] = ref_buffer[10];
    vtx_data[2].ref[3] = ref_buffer[11];

    vtx_data[3].x = vtx_data[2].x;
    vtx_data[3].y = vtx_data[1].y;
    vtx_data[3].u = vtx_data[2].u;
    vtx_data[3].v = vtx_data[1].v;
    vtx_data[3].ref[0] = ref_buffer[12];
    vtx_data[3].ref[1] = ref_buffer[13];
    vtx_data[3].ref[2] = ref_buffer[14];
    vtx_data[3].ref[3] = ref_buffer[15];

    /* Upload the array buffer. */
    glBufferData(GL_ARRAY_BUFFER, sizeof(vtx_fmt)*4, vtx_data, GL_DYNAMIC_DRAW);

    /* Free the array buffer */
    delete []vtx_data;

    //Prepare element data
    GLushort * ele_data = new GLushort[6];
    ele_data[0] = 0;
    ele_data[1] = 1;
    ele_data[2] = 2;
    ele_data[3] = 2;
    ele_data[4] = 1;
    ele_data[5] = 3;
    /* Upload the array buffer. */
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)*6, ele_data, GL_DYNAMIC_DRAW);

    /* Free the array buffer */
    delete []ele_data;

    /* Enable vertex array attribute 0 (position). */
    glEnableVertexAttribArray(VERTEX_ARRAY);
    glVertexAttribPointer(VERTEX_ARRAY, 2, GL_FLOAT, GL_FALSE, sizeof(vtx_fmt), 0);

    /* Enable vertex array attribute 1 (texture coordiate). */
    glEnableVertexAttribArray(TEXTURE_ARRAY);
    glVertexAttribPointer(TEXTURE_ARRAY, 2, GL_FLOAT, GL_FALSE, sizeof(vtx_fmt), (GLvoid *)(sizeof(GLfloat)*2));

    /* Enable vertex array attribute 1 (vpg reference). */
    glEnableVertexAttribArray(REF_ARRAY);
    glVertexAttribPointer(REF_ARRAY, 4, GL_FLOAT, GL_FALSE, sizeof(vtx_fmt), (GLvoid *)(sizeof(GLfloat)*4));

    return 6;
}

//-------------------------------------------------------------------------------
//target_width    : target image width
//target_height : target image height
//elenum        : the VPE element number for processing
GLvoid igProc(int target_width, int target_height, GLuint elenum)
{
    glViewport(0, 0, target_width, target_height);
    glDrawElements(GL_TRIANGLES, elenum, GL_UNSIGNED_INT, 0);
}

//-------------------------------------------------------------------------------
//target_width    : target image width
//target_height : target image height
//vpg_width        : virtual processing group width
//vpg_height    : virtual processing group height
//vpg_x            : virtual processing group x coordinate
//vpg_y            : virtual processing group y coordinate
GLvoid igProc(int target_width, int target_height, int vpg_width, int vpg_height, int vpg_x, int vpg_y)
{
    int offset_x = -1*vpg_x*target_width/vpg_width;
    int offset_y = -1*vpg_y*target_height/vpg_height;
    glViewport(offset_x, offset_y, target_width, target_height);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (GLvoid *)(sizeof(GLushort)*6*(vpg_x + vpg_width*vpg_y)));
}

//-------------------------------------------------------------------------------
//target_width    : target image width
//target_height : target image height
//vpg_width        : virtual processing group width
//vpg_height    : virtual processing group height
//vpg_x            : virtual processing group x coordinate
//vpg_y            : virtual processing group y coordinate
//new_vpg_x        : new virtual processing group x coordinate
//new_vpg_y        : new virtual processing group y coordinate
GLvoid igProc(int target_width, int target_height, int vpg_width, int vpg_height, int vpg_x, int vpg_y, int new_vpg_x, int new_vpg_y)
{
    int offset_x = (new_vpg_x-vpg_x)*target_width/vpg_width;
    int offset_y = (new_vpg_y-vpg_y)*target_height/vpg_height;
    glViewport(offset_x, offset_y, target_width, target_height);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (GLvoid *)(sizeof(GLushort)*6*(vpg_x + vpg_width*vpg_y)));
}

//-------------------------------------------------------------------------------
//target_width    : target image width
//target_height : target image height
//roi_x            : x start coordinate of region of interest
//roi_y            : y start coordinate of region of interest
GLvoid igProc(int target_width, int target_height, int roi_x, int roi_y)
{
    glViewport(-1*roi_x, -1*roi_y, target_width, target_height);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
}

//-------------------------------------------------------------------------------
//program        : the program handle id
//img_name        : the image name in sahder
//tex_unit        : the texture unit to bind, GL_TEXTURE0, GL_TEXTURE1, GL_TEXTURE2, GL_TEXTURE3,...
//texture        : the image handle id
GLvoid igProgramBindImage(GLuint program, const char * img_name, GLuint tex_unit, GLuint image)
{
    glActiveTexture(GL_TEXTURE0+tex_unit);
    glBindTexture(GL_TEXTURE_2D, image);
    glUniform1i(glGetUniformLocation(program, img_name), tex_unit);
}

//-------------------------------------------------------------------------------
//program        : the program handle id
//tex_unit        : the texture unit to bind, GL_TEXTURE0, GL_TEXTURE1, GL_TEXTURE2, GL_TEXTURE3,...
//image            : the image handle id
//param            : the image parameter for min filter, GL_NEAREST, GL_LINEAR
//param            : the image parameter for mag filter, GL_NEAREST, GL_LINEAR
GLvoid igImageParameter(GLuint image, GLuint tex_unit, GLint min_param, GLint mag_param)
{
    glActiveTexture(GL_TEXTURE0+tex_unit);
    glBindTexture(GL_TEXTURE_2D, image);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_param );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_param );
}

//-------------------------------------------------------------------------------
//shader        : the shader handle id
/*
IG_API GLvoid igFreeShader(GLuint shader)
{
    glDeleteShader(shader);
}
*/
//-------------------------------------------------------------------------------
//program        : the program handle id
/*
IG_API GLvoid igFreeProgram(GLuint program)
{
    glDeleteProgram(program);
}
*/
//-------------------------------------------------------------------------------
//n                : number of the texture handle to be free
//buffers        : image handle id pointer
/*
IG_API GLvoid igFreeImage(GLuint n, const GLuint* image)
{
    glDeleteTextures(n, image);
}
*/
//-------------------------------------------------------------------------------
//buffers        : The buffer pointer indicates both VPE and VPG handle id
/*
IG_API GLvoid igFreeVPE(const GLuint* buffers)
{
    glDeleteBuffers(2, buffers);
}
*/
//-------------------------------------------------------------------------------
//target_width    : target image width
//target_height : target image height
//target_ptr    : The pointer for target data storage
//format        : GL_RGBA, GL_LUMINANCE
IG_API GLvoid igReadTarget(GLuint target_width, GLuint target_height, void* target_ptr, GLenum format)
{
    glReadPixels(0, 0, target_width, target_height, format, GL_UNSIGNED_BYTE, target_ptr);
}

//-------------------------------------------------------------------------------
//target_width    : target image width
//target_height : target image height
//fbo            : return the frame buffer object id
//rbo            : return the render buffer object id
/*
IG_API bool igCreateBufferObject(int target_width, int target_height, GLuint &fbo, GLuint &rbo)
{
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8_OES, target_width, target_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        return 0;
    else
        return 1;
}
*/
//-------------------------------------------------------------------------------
//target_width    : target image width
//target_height : target image height
//fbo            : return the frame buffer object id
//texture        : for render to texture
//format        : texture format (not Luminance format)
IG_API bool igCreateBufferObject(int target_width, int target_height, GLuint &fbo, GLuint &texture, GLenum format)
{
    // allocate a texture name
    glGenTextures( 1, &texture );
    // select our current texture
    glBindTexture( GL_TEXTURE_2D, texture );
    glTexImage2D(GL_TEXTURE_2D, 0, format, target_width, target_height, 0, format, GL_UNSIGNED_BYTE, NULL);
    // when texture area is small, bilinear filter the closest MIP map
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    // when texture area is large, bilinear filter the first MIP map
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    // if wrap is true, the texture wraps over at the edges (repeat)
    //       ... false, the texture ends at the edges (clamp)
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);//GL_MIRRORED_REPEAT
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);//GL_MIRRORED_REPEAT

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);//render to texture


    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        return 0;
    else
        return 1;
}

//-------------------------------------------------------------------------------
//fbo            : return the frame buffer id
//rbo            : return the render buffer id
/*
IG_API GLvoid igFreeBufferObject(GLuint &fbo, GLuint &rbo)
{
    glDeleteFramebuffers(1, &fbo);
    glDeleteRenderbuffers(1, &rbo);
}
*/
//-------------------------------------------------------------------------------
//fbo            : return the frame buffer id
IG_API GLvoid igFreeBufferObject(GLuint &fbo)
{
    glDeleteFramebuffers(1, &fbo);
}
//-------------------------------------------------------------------------------
//filename    : for ppm image file
//mode        : GL_REPEAT, GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT
//xoffset    : the x dim start point reference to pixel (0,0)
//xoffset    : the y dim start point reference to pixel (0,0)
//swidth    : Sub image's width
//sheight    : Sub image's height
//Return    : the image handle id
IG_API GLuint igLoadSubImage( const char * filename, GLint mode, GLint xoffset, GLint yoffset, GLint swidth, GLint sheight)
{
    GLuint texture;
    unsigned char * data;
    unsigned char * sub_data;
    FILE * file;

    int width = 0xf0f0, height = 0xf0f0;

    // open texture data
    file = fopen(filename, "rb" );
    if ( file == NULL ) return 0;

    //Parsing the header
    bool bParsing = true;
    char ch, ch_buf[10];
    int i,j,k;
    i=0;
    while (bParsing)
    {
        ch = fgetc( file );
        if (ch==0x0A || ch==0x20)    //"\n" and " "
        {
            ch_buf[i] = '\0';
            i = 0;
            if (ch_buf[0] != 0x50)    //"P"
            {
                if (width==0xf0f0)
                {
                    width = atoi(ch_buf);
                }
                else if (height==0xf0f0)
                {
                    height = atoi(ch_buf);
                }
                else
                    bParsing = false;
            }
        }
        else
        {
            ch_buf[i] = ch;
            i++;
        }

    }
    // allocate buffer
    data = (unsigned char *)malloc( width * height * 3 );

    // read texture data
    for (j=height-1; j>=0; j--)
    {
        for (i=0; i<width; i++)
        {
            for (k=0; k<3; k++)
            {
                data[width*3*j + 3*i + k] = fgetc( file );
            }
        }
    }
    fclose( file );

    // Now extract sub image
    sub_data = (unsigned char *)malloc( swidth * sheight * 3 );
    for (j=0; j<sheight; j++)
    {
        for (i=0; i<swidth; i++)
        {
            for (k=0; k<3; k++)
            {
                sub_data[swidth*3*j + 3*i + k] = data[width*3*(yoffset+j) + 3*(xoffset+i) + k];
            }
        }
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);    //09,05,06 Eddie

    // allocate a texture name
    glGenTextures( 1, &texture );

    // select our current texture
    glBindTexture( GL_TEXTURE_2D, texture );

    // when texture area is small, bilinear filter the closest MIP map
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    // when texture area is large, bilinear filter the first MIP map
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    // if wrap is true, the texture wraps over at the edges (repeat)
    //       ... false, the texture ends at the edges (clamp)
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode );

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, swidth, sheight, 0, GL_RGB, GL_UNSIGNED_BYTE, sub_data);

    // free buffer
    free( data );
    free( sub_data );

    return texture;
}

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
IG_API GLuint igLoadSubImage( const char * filename, GLint mode , GLint width, GLint height, GLint xoffset, GLint yoffset, GLint swidth, GLint sheight)
{
    GLuint texture;
    unsigned char * data;
    FILE * file;

    // open texture data
    file = fopen(filename, "rb" );
    if ( file == NULL ) return 0;

    // allocate buffer
    data = (unsigned char *)malloc( width * height * 3 );

    // read texture data
    fread( data, width * height * 3, 1, file );
    fclose( file );

    // Now extract sub image
    unsigned char * sub_data = (unsigned char *)malloc( swidth * sheight * 3 );
    int i,j,k;
    for (j=0; j<sheight; j++)
    {
        for (i=0; i<swidth; i++)
        {
            for (k=0; k<3; k++)
            {
                sub_data[swidth*3*j + 3*i + k] = data[width*3*(yoffset+j) + 3*(xoffset+i) + k];
            }
        }
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);    //09,05,06 Eddie

    // allocate a texture name
    glGenTextures( 1, &texture );

    // select our current texture
    glBindTexture( GL_TEXTURE_2D, texture );

    // when texture area is small, bilinear filter the closest MIP map
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    // when texture area is large, bilinear filter the first MIP map
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    // if wrap is true, the texture wraps over at the edges (repeat)
    //       ... false, the texture ends at the edges (clamp)
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, swidth, sheight, 0, GL_RGB, GL_UNSIGNED_BYTE, sub_data);

    // free buffer
    free( data );
    free( sub_data );

    return texture;

}
//-------------------------------------------------------------------------------
//data        : for image data pointer
//mode        : GL_REPEAT, GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT
//format    : GL_RGB, GL_RGBA
//width        : image's width
//height    : image's height
//Return    : the image handle id
IG_API GLuint igLoadSubImage( void * data, GLint mode, GLenum format, int width, int height, GLint xoffset, GLint yoffset, GLint swidth, GLint sheight)
{
    GLuint texture;
    GLint ch = (format==GL_RGB) ? 3 : 4;

    height = 0;   	
    	
    unsigned char * sub_data = (unsigned char *)malloc( swidth * sheight * ch );
    unsigned char * ori_data = (unsigned char *)data;
    int i,j,k;
    for (j=0; j<sheight; j++)
    {
        for (i=0; i<swidth; i++)
        {
            for (k=0; k<ch; k++)
            {
                sub_data[swidth*ch*j + ch*i + k] = ori_data[width*ch*(yoffset+j) + ch*(xoffset+i) + k];
            }
        }
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);    //09,05,06 Eddie

    // allocate a texture name
    glGenTextures( 1, &texture );

    // select our current texture
    glBindTexture( GL_TEXTURE_2D, texture );

    // when texture area is small, bilinear filter the closest MIP map
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    // when texture area is large, bilinear filter the first MIP map
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    // if wrap is true, the texture wraps over at the edges (repeat)
    //       ... false, the texture ends at the edges (clamp)
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode );

    glTexImage2D(GL_TEXTURE_2D, 0, format, swidth, sheight, 0, format, GL_UNSIGNED_BYTE, sub_data);

    // free buffer
    free( sub_data );

    return texture;
}
//-------------------------------------------------------------------------------
//vs        : vertex shader handle
//fs        : fragment shader handle
//program    : current program handle
//Return    : the program handle id
IG_API GLvoid igUpdateShader(GLuint cur_shader, GLuint new_shader, GLuint program)
{
    glDetachShader(program, cur_shader);
    glAttachShader(program, new_shader);

    // Link the program
    glLinkProgram(program);
    glUseProgram(program);
}
//-------------------------------------------------------------------------------
//data        : for planar image data pointer
//mode        : GL_REPEAT, GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT
//format    : IG_YUV422P, IG_YUV420P, IG_YUV420SP
//width        : Y plan image's width
//height    : Y plan image's height
//plan        : the image plan handle pointer
//Return    : the image plan number
/*
IG_API GLuint igLoadImage(void * data, GLint mode, GLenum format, int width, int height, GLuint *plan)
{
    GLint pn = (format==IG_YUV420SP) ? 2 : 3;
    GLint cw = width>>1;
    GLint ch = (format!=IG_YUV422P) ? height>>1 : height;

    unsigned char * plan_data;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // allocate a texture name
    glGenTextures(pn, plan);

    //Create Luma plan
    plan_data = (unsigned char *)data;
    glBindTexture(GL_TEXTURE_2D, plan[0]);

    // when texture area is small, bilinear filter the closest MIP map
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    // when texture area is large, bilinear filter the first MIP map
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    // if wrap is true, the texture wraps over at the edges (repeat)
    //       ... false, the texture ends at the edges (clamp)
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode );

    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, plan_data);

    if (pn==3)
    {
        //Create Cb plan
        plan_data = plan_data + width*height;
        glBindTexture(GL_TEXTURE_2D, plan[1]);

        // when texture area is small, bilinear filter the closest MIP map
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        // when texture area is large, bilinear filter the first MIP map
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

        // if wrap is true, the texture wraps over at the edges (repeat)
        //       ... false, the texture ends at the edges (clamp)
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode );

        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, cw, ch, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, plan_data);

        //Create Cr plan
        plan_data = plan_data + cw*ch;
        glBindTexture(GL_TEXTURE_2D, plan[2]);

        // when texture area is small, bilinear filter the closest MIP map
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        // when texture area is large, bilinear filter the first MIP map
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

        // if wrap is true, the texture wraps over at the edges (repeat)
        //       ... false, the texture ends at the edges (clamp)
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode );

        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, cw, ch, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, plan_data);
    }
    else
    {
        //Create Chroma plan
        plan_data = plan_data + width*height;
        glBindTexture(GL_TEXTURE_2D, plan[1]);

        // when texture area is small, bilinear filter the closest MIP map
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        // when texture area is large, bilinear filter the first MIP map
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

        // if wrap is true, the texture wraps over at the edges (repeat)
        //       ... false, the texture ends at the edges (clamp)
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode );

        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, cw, ch, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, plan_data);
    }

    return pn;
}
*/
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
IG_API GLuint igLoadSubImage(void * data, GLint mode, GLenum format, int width, int height, GLint xoffset, GLint yoffset, GLint swidth, GLint sheight, GLuint *plan)
{
    GLint pn = (format==IG_YUV420SP) ? 2 : 3;
    GLint cw = width>>1;
    GLint ch = (format!=IG_YUV422P) ? height>>1 : height;
    GLint sw = swidth>>1;
    GLint sh = (format!=IG_YUV422P) ? sheight>>1 : sheight;
    GLint xo = xoffset>>1;
    GLint yo = (format!=IG_YUV422P) ? yoffset>>1 : yoffset;

    unsigned char * plan_data;
    unsigned char * sub_data = (unsigned char *)malloc( swidth * sheight );
    int i,j,k;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // allocate a texture name
    glGenTextures(pn, plan);

    //Create Luma plan
    plan_data = (unsigned char *)data;
    for (j=0; j<sheight; j++)
    {
        for (i=0; i<swidth; i++)
        {
            sub_data[swidth*j + i] = plan_data[width*(yoffset+j) + (xoffset+i)];
        }
    }
    glBindTexture(GL_TEXTURE_2D, plan[0]);

    // when texture area is small, bilinear filter the closest MIP map
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    // when texture area is large, bilinear filter the first MIP map
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    // if wrap is true, the texture wraps over at the edges (repeat)
    //       ... false, the texture ends at the edges (clamp)
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode );

    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, swidth, sheight, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, sub_data);

    if (pn==3)
    {
        //Create Cb plan
        plan_data = plan_data + width*height;
        for (j=0; j<sh; j++)
        {
            for (i=0; i<sw; i++)
            {
                sub_data[sw*j + i] = plan_data[cw*(yo+j) + (xo+i)];
            }
        }
        glBindTexture(GL_TEXTURE_2D, plan[1]);

        // when texture area is small, bilinear filter the closest MIP map
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        // when texture area is large, bilinear filter the first MIP map
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

        // if wrap is true, the texture wraps over at the edges (repeat)
        //       ... false, the texture ends at the edges (clamp)
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode );

        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, sw, sh, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, sub_data);

        //Create Cr plan
        plan_data = plan_data + cw*ch;
        for (j=0; j<sh; j++)
        {
            for (i=0; i<sw; i++)
            {
                sub_data[sw*j + i] = plan_data[cw*(yo+j) + (xo+i)];
            }
        }
        glBindTexture(GL_TEXTURE_2D, plan[2]);

        // when texture area is small, bilinear filter the closest MIP map
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        // when texture area is large, bilinear filter the first MIP map
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

        // if wrap is true, the texture wraps over at the edges (repeat)
        //       ... false, the texture ends at the edges (clamp)
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode );

        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, sw, sh, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, sub_data);
    }
    else
    {
        //Create Chroma plan
        plan_data = plan_data + width*height;
        for (j=0; j<sh; j++)
        {
            for (i=0; i<sw; i++)
            {
                for (k=0; k<2; k++)
                {
                    sub_data[sw*j*2 + i*2 + k] = plan_data[cw*(yo+j)*2 + (xo+i)*2 + k];
                }
            }
        }
        glBindTexture(GL_TEXTURE_2D, plan[1]);

        // when texture area is small, bilinear filter the closest MIP map
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        // when texture area is large, bilinear filter the first MIP map
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

        // if wrap is true, the texture wraps over at the edges (repeat)
        //       ... false, the texture ends at the edges (clamp)
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode );

        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, sw, sh, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, sub_data);
    }

    // free buffer
    free( sub_data );

    return pn;
}


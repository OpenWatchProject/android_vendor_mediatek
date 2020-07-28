#define MTK_LOG_ENABLE 1
#define LOG_TAG "CoreGlesWarp"
#include "coreGlesWarp.h"
#include "gles2_warp_shader.h"
#include "imagpu.h"
#include "libutility.h"
#define PINGPONG 2
//#define GL_TEXTURE_DISABLE_YUV_CONVERSION_XXX 0x8F67
//#define LOG_TAG "coreGlesWarp"
#define MAP_PRECISION 32
#include <sys/time.h>
static int start_sec, start_nsec;
static int end_sec, end_nsec;
static int total_time;

static void GetTime(int *sec, int *usec)
{
    timeval time;
    gettimeofday(&time, NULL);
    *sec = time.tv_sec;
    *usec = time.tv_usec;
}

#ifdef NEON_OPT
#include <arm_neon.h>
#endif

#include <EGL/eglext.h>

#include <stdio.h>
#include <string.h>
#ifdef SIM_MAIN
#define    MY_LOGD        printf
#else
#include <sys/system_properties.h>
#include <android/log.h>
#define MY_LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,##__VA_ARGS__)
//static int property_set(const char *key, const char *value)
//{
//    return __system_property_set(key, value);
//}
static int property_get(const char *key, char *value, const char *default_value)
{
    int len;
    len = __system_property_get(key, value);
    if(len > 0) {
       return len;
    }

    if(default_value) {
        len = strlen(default_value);
        memcpy(value, default_value, len + 1);
    }
    return len;
}

#endif

int count = 0;
/*
static void _dump_ppm(int width, int height, const char * filename, unsigned char * target_ptr, int format)
{
    int i = 0, j = 0;
    //    unsigned char *bytePtr, data[3];
    FILE* pFile;
    pFile = fopen(filename, "wb");

    if(format == 6)
        fprintf(pFile, "P6\n");
    else if(format ==5)
        fprintf(pFile, "P5\n");
    else
        return;

    fprintf(pFile, "%d %d\n255 ", width, height);

    /// P6 format
    int w, h;
    unsigned char *bufferPtr;
    unsigned char * buffer;
    if(format == 6)
    {
        buffer = new unsigned char [width * 3];

        //for (h = height-1; h >=0 ; h--)
        for (h = 0; h < height ; h++)
        {
            bufferPtr = buffer;
            for (w = 0; w < width; w++)
            {
                j = 4*width*h + 4*w;
                i = 3*w;
                bufferPtr[i] = target_ptr[j];
                bufferPtr[i+1] = target_ptr[j+1];
                bufferPtr[i+2] = target_ptr[j+2];
            }
            fwrite(buffer, 1, width * 3, pFile);
        }
        delete []buffer;
    }
    else if (format == 5)
    {
        buffer = new unsigned char [width];

        for (h = height-1; h >=0 ; h--)
        {
            bufferPtr = buffer;
            for (w = 0; w < width; w++)
            {
                j = width*h + w;
                i = w;
                bufferPtr[i] = target_ptr[j];
            }
            fwrite(buffer, 1, width, pFile);
        }
        delete []buffer;
    }
    fclose(pFile);

}
*/
static void CheckError(int t)
{
    GLuint egl_err = eglGetError();
    GLuint gl_err = glGetError();
    
    if(egl_err!=EGL_SUCCESS)
    {
        MY_LOGD("check point %d\n",t);
        MY_LOGD("Error code: egl %d\n",egl_err);
    }
    if(gl_err!=GL_NO_ERROR)
    {
        MY_LOGD("check point %d\n",t);
        MY_LOGD("Error code: gl %d\n",gl_err);
    }
}

void CoreGlesWarpExt::InitEGLImage(void)
{
    if(core_info.InitBufferType==0 ) //input
    {
      for(int i=0; i<(int)core_info.InitBufferCount; i++)
      {

#ifndef _MSC_VER

          //sp<GraphicBuffer>* bufAddr;
          //bufAddr = (sp<GraphicBuffer>*) *((int**)core_info.SrcGraphicBuffer + i);
          AHardwareBuffer* bufAddr;
          bufAddr = (AHardwareBuffer *) *((int**)core_info.SrcGraphicBuffer + i);

          Src[core_info.InputBufferHead].GB = bufAddr;

          //Src[core_info.InputBufferHead].GB =  *((sp<GraphicBuffer>*)core_info.SrcGraphicBuffer + i);

#endif
          Src[core_info.InputBufferHead].display = gles_display;
          Src[core_info.InputBufferHead].source = true;

          if(GLES_WARP_IMAGE_YV12  == core_info.ImgFmt)
              Src[core_info.InputBufferHead].is_yuv = true;
          else
              Src[core_info.InputBufferHead].is_yuv = false;
        
        Src[core_info.InputBufferHead].source_color_domain = core_info.source_color_domain;
        
        if(core_info.InputBufferHead < PINGPONG)
        {
          printf("core_info.InputBufferHead %d\n",core_info.InputBufferHead);
          GetEGLImageTexture(&Src[core_info.InputBufferHead]);
        }
        else
          GetEGLImage(&Src[core_info.InputBufferHead]);

        AHardwareBuffer_acquire(Src[core_info.InputBufferHead].GB);
        core_info.InputBufferHead++;
      }
    }
    if(core_info.InitBufferType==1) //output
    {
      for(int i=0; i<(int)core_info.InitBufferCount; i++)
      {

#ifndef _MSC_VER

          //sp<GraphicBuffer>* bufAddr;
          //bufAddr = (sp<GraphicBuffer>*) *((int**)core_info.SrcGraphicBuffer + i);
          AHardwareBuffer* bufAddr;
          bufAddr = (AHardwareBuffer *) *((int**)core_info.DstGraphicBuffer + i);
          Dst[core_info.OutputBufferHead].GB = bufAddr;

          //Dst[core_info.OutputBufferHead].GB =  *((sp<GraphicBuffer>*)core_info.DstGraphicBuffer + i);

#endif
          Dst[core_info.OutputBufferHead].display = gles_display;
          Dst[core_info.OutputBufferHead].source = false;
          Dst[core_info.OutputBufferHead].color_attachment = GL_COLOR_ATTACHMENT0;
          if(GLES_WARP_IMAGE_YV12  == core_info.OutImgFmt)
              Dst[core_info.OutputBufferHead].is_yuv = true;
          else
              Dst[core_info.OutputBufferHead].is_yuv = false;

        if(core_info.OutputBufferHead < PINGPONG)
          {printf("core_info.OutputBufferHead %d\n",core_info.OutputBufferHead);
          GetEGLImageTexture(&Dst[core_info.OutputBufferHead]);
          }
        else
          GetEGLImage(&Dst[core_info.OutputBufferHead]);

        AHardwareBuffer_acquire(Dst[core_info.OutputBufferHead].GB);
        core_info.OutputBufferHead++;
      }
    }
    CheckError(1);
}

void CoreGlesWarpExt::GlesWarpingInit(void)
{
    bool OK ;
    printf("init Warp %d\n",core_info.DisableEGLImageInit);
    if(core_info.DisableEGLImageInit != 1)
    {
      core_info.InputBufferHead = core_info.InputGBNum;
      core_info.OutputBufferHead = core_info.OutputGBNum;
    //setup source texture
    for(int i=0; i<(int)core_info.InputGBNum; i++)
    {
#ifndef _MSC_VER
          //sp<GraphicBuffer>* bufAddr;
          //bufAddr = (sp<GraphicBuffer>*) *((int**)core_info.SrcGraphicBuffer + i);
          AHardwareBuffer* bufAddr;
          bufAddr = (AHardwareBuffer *) *((int**)core_info.SrcGraphicBuffer + i);

        Src[i].GB = bufAddr;
        //unsigned int value = (unsigned int)Src[i].GB;
        MY_LOGD("Src[i].GB addr %lx\n", (long)bufAddr);
#endif
        Src[i].display = gles_display;
        Src[i].source = true;

        if(GLES_WARP_IMAGE_YV12  == core_info.ImgFmt)
            Src[i].is_yuv = true;
        else
            Src[i].is_yuv = false;

        Src[i].source_color_domain = core_info.source_color_domain;
        GetEGLImageTexture(&Src[i]);
        AHardwareBuffer_acquire(Src[i].GB);
    }

    //setup output image
    for(int i=0; i<(int)core_info.OutputGBNum; i++)
    {
#ifndef _MSC_VER
          //sp<GraphicBuffer>* bufAddr;
          //bufAddr = (sp<GraphicBuffer>*) *((int**)core_info.SrcGraphicBuffer + i);
          AHardwareBuffer* bufAddr;
          bufAddr = (AHardwareBuffer *) *((int**)core_info.DstGraphicBuffer + i);
        Dst[i].GB = bufAddr;
        //unsigned int value = (unsigned int)Dst[i].GB;
        MY_LOGD("Dst[i].GB addr %lx\n",(long)bufAddr);
#endif
        Dst[i].display = gles_display;
        Dst[i].source = false;
        Dst[i].color_attachment = GL_COLOR_ATTACHMENT0;

        if(GLES_WARP_IMAGE_YV12  == core_info.OutImgFmt)
            Dst[i].is_yuv = true;
        else
            Dst[i].is_yuv = false;
        GetEGLImageTexture(&Dst[i]);
        AHardwareBuffer_acquire(Dst[i].GB);
    }
    }
    else
    {
      core_info.InputBufferHead = 0;
      core_info.OutputBufferHead = 0;
    }

    //create shader program
    if(core_info.ImgFmt == GLES_WARP_IMAGE_YV12)
        OK = CreateShderProgram(gles2_warp_vs, gles2_warp_fs_ext);
    else
        OK = CreateShderProgram(gles2_warp_vs, gles2_warp_fs);
    if(!OK)
    {
        MY_LOGD("coreGlesWarp -- GlesWarpingInit fail");
    }

    core_info.PreWarpMapSize[0]=0;
    core_info.PreWarpMapSize[1]=0;
}

void CoreGlesWarpExt::GlesWarpingMain(void)
{
    bool OK;
    printf("CoreGlesWarpExt::GlesWarpingMain \n");
    OK = GlesWarping();
    if(!OK)
    {
        MY_LOGD("coreGlesWarp -- GlesWarpingMain fail");
    }
}

void CoreGlesWarpExt::GlesWarpingReset(void)
{
    bool OK;

    //clean source GB
    for(int i=0; i<(int)core_info.InputBufferHead; i++){
        if(i<PINGPONG)
          DestroyEGLImageTexture(&Src[i]);
        else
          DestroyEGLImage(&Src[i]);
        //(Src[i].GB) = NULL;
        AHardwareBuffer_release(Src[i].GB);
    }
    //clean output GB
    for(int i=0; i<(int)core_info.OutputBufferHead; i++){
        if(i<PINGPONG)
          DestroyEGLImageTexture(&Dst[i]);
        else
          DestroyEGLImage(&Dst[i]);
        //(Dst[i].GB) = NULL;
        AHardwareBuffer_release(Dst[i].GB);
    }

    //clean shader obj
    OK = DestroyShaderProgram();
    if(!OK)
    {
        MY_LOGD("coreGlesWarp -- GlesWarpingReset fail");
    }
}

bool CoreGlesWarpExt::CreateShderProgram(const char*vs, const char*fs)
{
    bool OK = true;
    bool NoOK = false;
    /*
        Load shader
    */

    VertShader = igLoadShader(vs, GL_VERTEX_SHADER);
    bool vs_state = igCheckCompile(VertShader);
    if (!VertShader || !vs_state)
    {
        MY_LOGD("create vertex shader fail!\n\n");
        return NoOK;
    }

    FragShader = igLoadShader(fs, GL_FRAGMENT_SHADER);
    bool ps_state = igCheckCompile(FragShader);
    if (!FragShader || !ps_state)
    {
        MY_LOGD("create fragement shader fail!\n\n");
        return NoOK;
    }

    /*
        Create shader program
    */

    ProgramObject = igCreateProgram(VertShader, FragShader, 0);
    bool pr_state = igCheckLink(ProgramObject);
    if (!ProgramObject || !pr_state)
    {
        MY_LOGD("create shader program fail!\n\n");
        return NoOK;
    }

    //off-screen buffer

    //if(!igCreateBufferObject(target_width, target_height, fbo, rbo))
    //{
    //    printf("create output result buffer fail!\n\n");
    //    return NoOK;
    //}

    /*
        Create Mesh
    */
    //GLuint elenum = igCreateVPE(target_width, target_height, 1 ,1, uiVBO);
    //elenum = igCreateVPE(target_width, target_height, warp_width, warp_height, uiVBO);

    elenum = igCreateVPE(core_info.Width, core_info.Height, core_info.MaxWarpMapSize[0]-1, core_info.MaxWarpMapSize[1]-1, uiVBO);
    core_info.PreWarpMapSize[0] = core_info.MaxWarpMapSize[0];
    core_info.PreWarpMapSize[1] = core_info.MaxWarpMapSize[1];
    MY_LOGD("+++create vpg %d %d\n",core_info.MaxWarpMapSize[0], core_info.MaxWarpMapSize[1]);
    core_info.FWarpMap = (float*)malloc(sizeof(float)*(core_info.MaxWarpMapSize[0])* (core_info.MaxWarpMapSize[1]));

    ++count;

    return OK;
}

bool CoreGlesWarpExt::DestroyShaderProgram()
{
    bool OK = true;
    //bool NoOK = false;

    igFreeProgram(ProgramObject);
    igFreeShader(FragShader);
    igFreeShader(VertShader);
    //igFreeBufferObject(fbo);
    //igFreeBufferObject(rbo);
    igFreeVPE(uiVBO);
    free(core_info.FWarpMap);
    --count;
    MY_LOGD("GPU debug count %d",count);
    return OK;
}

bool CoreGlesWarpExt::GlesWarping()
{
    bool OK = true;
    bool NoOK = false;

    int source_width = core_info.Width;
    int source_height = core_info.Height;
    int GB_width = core_info.GB_Width;
    int GB_height = core_info.GB_Height;
    int clip_width = core_info.ClipWidth;
    int clip_height = core_info.ClipHeight;
    //int grid_width = core_info.MaxWarpMapSize[0];
    //int grid_height = core_info.MaxWarpMapSize[1];
    int clipX = 0;//core_info.ClipX[0];
    int clipY = 0;//core_info.ClipY[0];
    int warp_map_num = 1;//core_info.WarpMapNum;
    int version = core_info.TuningPara.GLESVersion;
    //MY_LOGD("GlesWarping context: %d",version);
    GLfloat WarpMapSizeX[MAX_WARP_MAP_NUM];
    GLfloat WarpMapSizeY[MAX_WARP_MAP_NUM];
    GLenum src_target, dst_target;
    //get input texture pointer
    int current_input = 0;
    int current_output = 0;
#ifndef _MSC_VER
    current_input = -1;
    current_output = -1;
    printf("core_info.InputBufferHead %d, core_info.OutputBufferHead %d\n",core_info.InputBufferHead,core_info.OutputBufferHead);
    for(int i=0; i< (int)core_info.InputBufferHead; i++)
        //if(Src[i].GB == *((sp<GraphicBuffer>*)core_info.SrcGraphicBuffer))
        if((Src[i].GB) == ((AHardwareBuffer *)core_info.SrcGraphicBuffer))
            { //unsigned int value = (unsigned int)Src[i].GB;
                //MY_LOGD("matching Src[i].GB %d\n", value);
                current_input = i;
            }
    //get output texture pointer
    for(int i=0; i< (int)core_info.OutputBufferHead; i++)
        //if(Dst[i].GB == *((sp<GraphicBuffer>*)core_info.DstGraphicBuffer))
        if((Dst[i].GB) == ((AHardwareBuffer *)core_info.DstGraphicBuffer))
            {//unsigned int value = (unsigned int)Dst[i].GB;
                //MY_LOGD("matching Dst[i].GB %d\n", value);
            current_output = i;
            }
    if((current_input == -1) || (current_output == -1))
    {
        MY_LOGD("coreGlesWarp -- Wrong Graphic Buffer: input %d, output %d",current_input,current_output);
        return NoOK;
    }
    else
      printf("current input %d, current output %d\n",current_input, current_output);

#endif
    if(Src[current_input].is_yuv == true)
      src_target = GL_TEXTURE_EXTERNAL_OES;
    else
      src_target = GL_TEXTURE_2D;

    if(Dst[current_output].is_yuv == true)
      dst_target = GL_TEXTURE_EXTERNAL_OES;
    else
      dst_target = GL_TEXTURE_2D;

    if((core_info.PreWarpMapSize[0]!= core_info.WarpMapSize[0][0]) || (core_info.PreWarpMapSize[1]!= core_info.WarpMapSize[0][1]))
    {
        igFreeVPE(uiVBO);
        elenum = igCreateVPE(core_info.Width, core_info.Height, core_info.WarpMapSize[0][0]-1, core_info.WarpMapSize[0][1]-1, uiVBO);
        core_info.PreWarpMapSize[0] = core_info.WarpMapSize[0][0];
        core_info.PreWarpMapSize[1] = core_info.WarpMapSize[0][1];
        MY_LOGD("Warp map size changed");
        free(core_info.FWarpMap);
        core_info.FWarpMap = (float*)malloc(sizeof(float)*(core_info.WarpMapSize[0][0])* (core_info.WarpMapSize[0][1]));
    }
    //MY_LOGD("create max vpg %d %d\n",core_info.MaxWarpMapSize[0], core_info.MaxWarpMapSize[1]);
    //MY_LOGD("create vpg %d %d\n",core_info.WarpMapSize[0][0], core_info.WarpMapSize[0][1]);

    /*
        Load warp field
    */

    GLuint warpX_tex_obj[MAX_WARP_MAP_NUM], warpY_tex_obj[MAX_WARP_MAP_NUM];
    if(warp_map_num > MAX_WARP_MAP_NUM)
    {
        MY_LOGD("coreGlesWarp -- input WarpMap overflow");
        return NoOK;
    }

    char dft[32];
    char value[128] = {'\0'};
    int demoModeEanbled;

    sprintf(dft, "%d", 0);
    property_get("vendor.debug.eis.demo", value, dft);
    demoModeEanbled = atoi(value);
    //demo mode left part
    if(demoModeEanbled!=0)
    {
        int demoGridSize = 2;
        //create element array for 2x2 grid
        elenum = igCreateVPE(core_info.Width, core_info.Height, demoGridSize-1, demoGridSize-1, uiVBO);
        float x_offset = ( source_width  - clip_width  ) / 2;
        float y_offset = ( source_height - clip_height ) / 2;
        core_info.FWarpMap[0] =(float)( x_offset )*MAP_PRECISION;
        core_info.FWarpMap[1] =(float)( x_offset+(clip_width-1) )*MAP_PRECISION;
        core_info.FWarpMap[2] =(float)( x_offset )*MAP_PRECISION;
        core_info.FWarpMap[3] =(float)( x_offset+(clip_width-1) )*MAP_PRECISION;

        warpX_tex_obj[0] = igLoadFloatingImage( core_info.FWarpMap, GL_MIRRORED_REPEAT, demoGridSize, demoGridSize, version);

        core_info.FWarpMap[0] =(float)( y_offset )*MAP_PRECISION;
        core_info.FWarpMap[1] =(float)( y_offset )*MAP_PRECISION;
        core_info.FWarpMap[2] =(float)( y_offset+(clip_height-1) )*MAP_PRECISION;
        core_info.FWarpMap[3] =(float)( y_offset+(clip_height-1) )*MAP_PRECISION;

        warpY_tex_obj[0] = igLoadFloatingImage( core_info.FWarpMap, GL_MIRRORED_REPEAT, demoGridSize, demoGridSize, version);
        MY_LOGD("core_info.WarpMapSize[i][0] %d core_info.WarpMapSize[i][1] %d\n",core_info.WarpMapSize[0][0], core_info.WarpMapSize[0][1]);

        WarpMapSizeX[0] = (float)core_info.WarpMapSize[0][0];
        WarpMapSizeY[0] = (float)core_info.WarpMapSize[0][1];

        /*
            uniforms
        */

        glUseProgram(ProgramObject);//must!

        int i32Location1 = glGetUniformLocation(ProgramObject, "source_width");
        glUniform1f(i32Location1, (GLfloat)(source_width-1)*MAP_PRECISION);
        int i32Location2 = glGetUniformLocation(ProgramObject, "source_height");
        glUniform1f(i32Location2, (GLfloat)(source_height-1)*MAP_PRECISION);
        int i32Location3 = glGetUniformLocation(ProgramObject, "ori_source_width");
        glUniform1f(i32Location3, (GLfloat)(source_width-1));
        int i32Location4 = glGetUniformLocation(ProgramObject, "ori_source_height");
        glUniform1f(i32Location4, (GLfloat)(source_height-1));
        int i32Location5 = glGetUniformLocation(ProgramObject, "buffer_width");
        glUniform1f(i32Location5, (GLfloat)(GB_width-1));
        int i32Location6 = glGetUniformLocation(ProgramObject, "buffer_height");
        glUniform1f(i32Location6, (GLfloat)(GB_height-1));

        int i32Location7 = glGetUniformLocation(ProgramObject, "boundx");
        glUniform1f(i32Location7, ((GLfloat)(source_width-2))/(GLfloat)(GB_width-1) );
        int i32Location8 = glGetUniformLocation(ProgramObject, "boundy");
        glUniform1f(i32Location8, ((GLfloat)(source_height-2))/(GLfloat)(GB_height-1) );

        MY_LOGD("GPU source_width, height %d %d",source_width, source_height);
        MY_LOGD("GPU GB_width, height %d %d",GB_width, GB_height);

        /*
            Trigger GPU
        */
        if(core_info.DisableEGLImageInit!=1)
        {
          glBindFramebuffer(GL_FRAMEBUFFER,Dst[current_output].fbo);//
        }
        else
        {
          glActiveTexture(GL_TEXTURE0);
          glBindTexture(dst_target, Dst[current_output%PINGPONG].textureID);
          glEGLImageTargetTexture2DOES(dst_target, Dst[current_output].img);
          glBindFramebuffer(GL_FRAMEBUFFER,Dst[current_output%PINGPONG].fbo);//
        }


        //glViewport(-1*clipX, -1*clipY, clip_width, clip_height);

        //glViewport(-1*clipX, -1*clipY, source_width , source_height); ==> original
        //left part
        glViewport(0,0,clip_width/2, clip_height);

        MY_LOGD("clip_width %d, clip_height %d\n",clip_width, clip_height);
        //glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT);

        MY_LOGD("Dst[%d].fbo = %d\n", current_output,Dst[current_output].fbo);

        glUseProgram(ProgramObject); //
        glActiveTexture(GL_TEXTURE0+1);
        if(core_info.DisableEGLImageInit!=1)
        {
            glBindTexture(src_target, Src[current_input].textureID);
        }
        else
        {
            glBindTexture(src_target, Src[current_input%PINGPONG].textureID);
        }

        if(Dst[current_output].is_yuv == true )
        {
            //MY_LOGD("GPU Dst is yuv\n");
            //glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_DISABLE_YUV_CONVERSION_XXX, GL_TRUE); //True: Shader output yuv444
        }
        else
        {
            //MY_LOGD("GPU Dst is rgba\n");
        }
#ifndef _MSC_VER
        glEGLImageTargetTexture2DOES(src_target, Src[current_input].img);
#endif
        glUniform1i(glGetUniformLocation(ProgramObject, "img"), 1);

        MY_LOGD("Src[%d].textureID = %d\n",current_input,Src[current_input].textureID);
        igProgramBindImage(ProgramObject, "warp_x_img", 2, warpX_tex_obj[0]);
        igProgramBindImage(ProgramObject, "warp_y_img", 3, warpY_tex_obj[0]);
        glDrawElements(GL_TRIANGLES, elenum, GL_UNSIGNED_INT, 0);
        //re-generate element array for right part
        elenum = igCreateVPE(core_info.Width, core_info.Height, core_info.WarpMapSize[0][0]-1, core_info.WarpMapSize[0][1]-1, uiVBO);
    }


    for(int i=0; i<warp_map_num; i++)
    {
        /*
        int IW = clip_width;
        int IH = clip_height;
        int IWs = core_info.WarpMapSize[i][0];
        int IHs = core_info.WarpMapSize[i][1];
        for(int j=0; j<IHs; j++)
        {
            int* pWarp = (int*)core_info.WarpMapAddr[i][0] + j*IWs;
            for(int ii=0; ii<IWs; ii++)
            {
                *pWarp++ = (int)(((float)ii/(IWs-1))*MAP_PRECISION*(IW-1)+0.5);
            }
        }
        for(int j=0; j<IHs; j++)
        {
            int* pWarp = (int*)core_info.WarpMapAddr[i][1] + j*IWs;
            int tmp = (int)(((float)j/(IHs-1))*MAP_PRECISION*(IH-1)+0.5);
            for(int ii=0; ii<IWs; ii++)
            {
                *pWarp++ = tmp;
            }
        }
        */
        int* ptr;
        ptr = (int*)core_info.WarpMapAddr[i][0];

#ifdef NEON_OPT

        float* fptr;
        int boundary =  ((core_info.WarpMapSize[i][0])*(core_info.WarpMapSize[i][1]))%4;
        int nloop =  ((core_info.WarpMapSize[i][0])*(core_info.WarpMapSize[i][1]))/4;
        float32x4_t vftemp;
        int32x4_t vitemp;

        fptr = core_info.FWarpMap;
        for(int j=0; j<nloop; j++, ptr+=4, fptr+=4)
        {
            vitemp = vld1q_s32(ptr);
            vftemp = vcvtq_f32_s32(vitemp);
            vst1q_f32(fptr,vftemp);
        }
        for(int j=0; j<boundary; j++, fptr++)
            *fptr = (float) *ptr++;
#else
        for(int j=0; j< (core_info.WarpMapSize[i][0])*(core_info.WarpMapSize[i][1]); j++)
        {
            core_info.FWarpMap[j] =(float) *ptr++;
        }
#endif

        warpX_tex_obj[i] = igLoadFloatingImage( core_info.FWarpMap, GL_MIRRORED_REPEAT, core_info.WarpMapSize[i][0], core_info.WarpMapSize[i][1], version);

        ptr = (int*)core_info.WarpMapAddr[i][1];

#ifdef NEON_OPT
        fptr = core_info.FWarpMap;
        for(int j=0; j<nloop; j++, ptr+=4, fptr+=4)
        {
            vitemp = vld1q_s32(ptr);
            vftemp = vcvtq_f32_s32(vitemp);
            vst1q_f32(fptr,vftemp);
        }
        for(int j=0; j<boundary; j++, fptr++)
            *fptr = (float) *ptr++;
#else
        for(int j=0; j< (core_info.WarpMapSize[i][0])*(core_info.WarpMapSize[i][1]); j++)
        {
            core_info.FWarpMap[j] =(float) *ptr++;
        }
#endif
        warpY_tex_obj[i] = igLoadFloatingImage( core_info.FWarpMap, GL_MIRRORED_REPEAT, core_info.WarpMapSize[i][0], core_info.WarpMapSize[i][1], version);
        //MY_LOGD("core_info.WarpMapSize[i][0] %d core_info.WarpMapSize[i][1] %d\n",core_info.WarpMapSize[i][0], core_info.WarpMapSize[i][1]);
        /*
        FILE* fp;
        char in_warp[50];
        sprintf(in_warp,"sdcard/warpXY_%d.bin",core_info.TuningPara.demo);
        fp = fopen(in_warp,"wb");
        fwrite((void*)core_info.WarpMapAddr[i][0], sizeof(int), core_info.WarpMapSize[i][0]*core_info.WarpMapSize[i][1],fp);
        //fclose(fp);
        //sprintf(in_warp,"sdcard/warpY_%d.bin",count);
        //fp = fopen(in_warp,"wb");
        fwrite((void*)core_info.WarpMapAddr[i][1], sizeof(int), core_info.WarpMapSize[i][0]*core_info.WarpMapSize[i][1],fp);
        fclose(fp);
        */
        WarpMapSizeX[i] = (float)core_info.WarpMapSize[i][0];
        WarpMapSizeY[i] = (float)core_info.WarpMapSize[i][1];
    }
//CheckError(0);
    /*
        uniforms
    */

    glUseProgram(ProgramObject);//must!

    int i32Location1 = glGetUniformLocation(ProgramObject, "source_width");
    glUniform1f(i32Location1, (GLfloat)(source_width-1)*MAP_PRECISION);
    int i32Location2 = glGetUniformLocation(ProgramObject, "source_height");
    glUniform1f(i32Location2, (GLfloat)(source_height-1)*MAP_PRECISION);
    int i32Location3 = glGetUniformLocation(ProgramObject, "ori_source_width");
    glUniform1f(i32Location3, (GLfloat)(source_width-1));
    int i32Location4 = glGetUniformLocation(ProgramObject, "ori_source_height");
    glUniform1f(i32Location4, (GLfloat)(source_height-1));
    int i32Location5 = glGetUniformLocation(ProgramObject, "buffer_width");
    glUniform1f(i32Location5, (GLfloat)(GB_width-1));
    int i32Location6 = glGetUniformLocation(ProgramObject, "buffer_height");
    glUniform1f(i32Location6, (GLfloat)(GB_height-1));

    int i32Location7 = glGetUniformLocation(ProgramObject, "boundx");
    glUniform1f(i32Location7, ((GLfloat)(source_width-2))/(GLfloat)(GB_width-1) );
    int i32Location8 = glGetUniformLocation(ProgramObject, "boundy");
    glUniform1f(i32Location8, ((GLfloat)(source_height-2))/(GLfloat)(GB_height-1) );

    int i32Location9 = glGetUniformLocation(ProgramObject, "yuv_format");
    if(Dst[current_output].is_yuv == true )
         glUniform1i(i32Location9, (GLint) 0);
    else
         glUniform1i(i32Location9, (GLint) Src[current_input%PINGPONG].source_color_domain);


    //MY_LOGD("GPU source_width, height %d %d",source_width, source_height);
    //MY_LOGD("GPU GB_width, height %d %d",GB_width, GB_height);
    //int i32Location3 = glGetUniformLocation(ProgramObject, "warp_level");
    //glUniform1f(i32Location3, (GLfloat)core_info.WarpLevel);
//CheckError(1);
    /*
        Transform matrix
    */
    /*
    if(core_info.WarpMatrixNum > MAX_WARP_MATRIX_NUM)
    {
        MY_LOGD("coreGlesWarp -- core_info.WarpMatrixNum > MAX_WARP_MATRIX_NUM");
        return NoOK;
    }
    float pWrapMatrix[MAX_WARP_MATRIX_NUM*9];
    for(int i=0; i<core_info.WarpMatrixNum; i++)
    {
        pWrapMatrix[i*9]= core_info.Hmtx[i][0];    pWrapMatrix[i*9+3]= core_info.Hmtx[i][1];    pWrapMatrix[i*9+6]= core_info.Hmtx[i][2];
        pWrapMatrix[i*9+1]= core_info.Hmtx[i][3]; pWrapMatrix[i*9+4]= core_info.Hmtx[i][4];    pWrapMatrix[i*9+7]= core_info.Hmtx[i][5];
        pWrapMatrix[i*9+2]= core_info.Hmtx[i][6];            pWrapMatrix[i*9+5]= core_info.Hmtx[i][7];    pWrapMatrix[i*9+8]= core_info.Hmtx[i][8];

    }
    GLint i32LocMat = glGetUniformLocation(ProgramObject, "matrix");
    glUniformMatrix3fv(i32LocMat, core_info.WarpMatrixNum, GL_FALSE, pWrapMatrix);
    */

    /*
        Trigger GPU
    */

    if(core_info.DisableEGLImageInit!=1)
    {
      glBindFramebuffer(GL_FRAMEBUFFER,Dst[current_output].fbo);//
    }
    else
    {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(dst_target, Dst[current_output%PINGPONG].textureID);
      glEGLImageTargetTexture2DOES(dst_target, Dst[current_output].img);
      glBindFramebuffer(GL_FRAMEBUFFER,Dst[current_output%PINGPONG].fbo);//
    }

    //glViewport(-1*clipX, -1*clipY, clip_width, clip_height);
    if(demoModeEanbled)
    {
        glViewport(clip_width/2,0,clip_width/2, clip_height);
    }
    else
    {
        glViewport(-1*clipX, -1*clipY, clip_width , clip_height);//+(clip_height/2)
    }

//MY_LOGD("clip_width %d, clip_height %d\n",clip_width, clip_height);
    //glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    //glClear(GL_COLOR_BUFFER_BIT);
//MY_LOGD("Dst[%d].fbo = %d\n", current_output,Dst[current_output].fbo);

    glUseProgram(ProgramObject); //
    glActiveTexture(GL_TEXTURE0+1);

    if(core_info.DisableEGLImageInit!=1)
    {
      glBindTexture(src_target, Src[current_input].textureID);//GL_TEXTURE_EXTERNAL_OES
    }
    else
    {
      glBindTexture(src_target, Src[current_input%PINGPONG].textureID);//GL_TEXTURE_EXTERNAL_OES
    }

    if(Dst[current_output].is_yuv == true )
    {
        //MY_LOGD("GPU Dst is yuv\n");
        //glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_DISABLE_YUV_CONVERSION_XXX, GL_TRUE); //True: Shader output yuv444
    }
    else
    {
        //MY_LOGD("GPU Dst is rgba\n");
    }

#ifndef _MSC_VER
    glEGLImageTargetTexture2DOES(src_target, Src[current_input].img);
#endif
    glUniform1i(glGetUniformLocation(ProgramObject, "img"), 1);

//MY_LOGD("Src[%d].textureID = %d\n",current_input,Src[current_input].textureID);
//CheckError(2);
    igProgramBindImage(ProgramObject, "warp_x_img", 2, warpX_tex_obj[0]);
    igProgramBindImage(ProgramObject, "warp_y_img", 3, warpY_tex_obj[0]);
//CheckError(3);
GetTime(&start_sec, &start_nsec);
    glDrawElements(GL_TRIANGLES, elenum, GL_UNSIGNED_INT, 0);

//#ifndef _MSC_VER
EGLSyncKHR sync = eglCreateSyncKHR(gles_display, EGL_SYNC_FENCE_KHR, NULL);
EGLint result = eglClientWaitSyncKHR(gles_display, sync, 0, EGL_FOREVER_KHR);
 if (result == EGL_FALSE) {
      MY_LOGD("EGL FENCE: error waiting for fence: %#x\n",eglGetError());
      return NoOK;
    }
   eglDestroySyncKHR(gles_display, sync);
//#endif
   GetTime(&end_sec, &end_nsec);
   total_time = (end_sec - start_sec) * 1000000 + (end_nsec - start_nsec);
   printf("gpu draw time %.1f ms\n", total_time/1000.0);


//CheckError(4);
/*
    unsigned char* pixels = (unsigned char*)malloc(4*clip_width*clip_height);
    glReadPixels(0, 0, clip_width, clip_height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    char debug_out[50];
    sprintf(debug_out,"sdcard/readpixel_gb_%d.ppm",core_info.TuningPara.demo);
    count++;
    _dump_ppm(clip_width, clip_height, debug_out, pixels, 6);
//MY_LOGD("pixel %d %d %d %d\n",pixels[0],pixels[1],pixels[2],pixels[3]);
    free(pixels);
    */
//CheckError(5);



    /*
        Step 7 - Finish all the GPU task
        Delete all the handle to prevent memory leakage
    */
    igFreeImage(warp_map_num, warpX_tex_obj);
    igFreeImage(warp_map_num, warpY_tex_obj);


    //igFreeVPE(uiVBO);


    return OK;
}

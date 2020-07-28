

#include "SkBitmap.h"
#include "SkPaint.h"
#include "SkCanvas.h"
#include "SkStream.h"
#include "SkColorPriv.h"
#include "SkString.h"
#include "SkImageEncoder.h"
#include "SkImageDecoder.h"

static void saveFile(const char name[], const SkBitmap& bm) {
    SkBitmap copy;
    if (!bm.copyTo(&copy, SkBitmap::kARGB_8888_Config)) {
        return;
    }
    
    //SkImageEncoder::EncodeFile(name, copy, SkImageEncoder::kPNG_Type, 100);
    SkImageEncoder::EncodeFile(name, copy, SkImageEncoder::kJPEG_Type, 100);
}


static void erase(SkBitmap& bm) {
    if (bm.config() == SkBitmap::kA8_Config) {
        bm.eraseColor(0);
    } else {
        bm.eraseColor(SK_ColorWHITE);
    }
}

#include <time.h>
static double intervalInMSec(const timespec start_clock
                           , const timespec end_clock)
{
    double duration_clock;
    if ((end_clock.tv_nsec - start_clock.tv_nsec) < 0) {
        duration_clock = (end_clock.tv_sec - start_clock.tv_sec-1)*1000;
        duration_clock += (1000000000 + end_clock.tv_nsec - start_clock.tv_nsec)
                           / 1000000.0;
    } else {
        duration_clock = (end_clock.tv_sec - start_clock.tv_sec)*1000;
        duration_clock += (end_clock.tv_nsec - start_clock.tv_nsec) / 1000000.0;
    }
    return duration_clock;
}


#define BUFFER_SIZE (1920*1080*4) 

int main(int argc, char* argv[])
{
  FILE *fp;
	//SkBitmap srcImage;
	char file[128] = "/data/otis/test.jpg";
	
	unsigned char *src_va;
	unsigned char *dst_va;
    int sampleSize = 1;
    int preferSize = 0;
    int postproc = 0; 
    int postprocflag = 0;

    SkImageDecoder::Mode mode = SkImageDecoder::kDecodePixels_Mode;
    SkBitmap::Config prefConfig = SkBitmap::kARGB_8888_Config;

    bool doDither = true;
    bool isMutable = false;
    //bool willScale = applyScale && scale != 1.0f;
    bool isPurgeable = 0;//!willScale && (forcePurgeable || (allowPurgeable && optionsPurgeable(env, options)));
    bool preferQualityOverSpeed = false;  
    unsigned int times = 1 ;
    unsigned int i = 0;
    SkBitmap* decoded = NULL;
    
    for(i=0; i<times;i++){
      if(decoded != NULL)
        delete decoded ;
      decoded  = new SkBitmap;
      
    timespec start_cpu;
    timespec end_cpu;
    if (-1 == clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_cpu)) {
        timespec none = {0, 0};
        start_cpu = none;
    }
      
      SkFILEStream    stream(file);
      if (!stream.isValid()) {
        printf("no file found!!!\n");
        return false ;
        
          //if (SkImageDecoder::DecodeStream(&stream, bm, pref, mode, format)) {
          //    bm->pixelRef()->setURI(file);
          //    return true;
          //}
      }
      
      
      SkImageDecoder* decoder = SkImageDecoder::Factory(&stream);
      if (decoder == NULL) {
        printf("no decoder found!!!\n");
          return false ;//nullObjectReturn("SkImageDecoder::Factory returned null");
      }
      
      decoder->setSampleSize(sampleSize);
      decoder->setDitherImage(doDither);
      decoder->setPreferQualityOverSpeed(preferQualityOverSpeed);
      decoder->setPreferSize(preferSize);
      //decoder->setPostProcFlag((postproc | (postprocflag << 4)));
      
      
      if (!decoder->decode(&stream, decoded, prefConfig, mode, false)) {
           printf("no decode fail!!!\n");
          return false ;//nullObjectReturn("decoder->decode returned false");
      }

      if (-1 == clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_cpu)) 
      {
          timespec none = {0, 0};
          end_cpu = none;
      }      
      double interval = intervalInMSec(start_cpu, end_cpu);
      printf("interval = %lf\n", interval);      
      
      saveFile("/data/otis/output.jpg", *decoded);

      delete decoder ;
   }
    
    //src_va = (unsigned char *)malloc(BUFFER_SIZE);
    //if(src_va == NULL)
    //{
    //    //xlog("Can not allocate memory\n");
    //    return -1;
    //}
    //fp = fopen("/data/otis/dump.raw", "r");
    //fread(src_va , 1 , BUFFER_SIZE , fp);
    //fclose(fp);



	
	//SkImageDecoder::DecodeFile("/data/otis/test.jpg", &srcImage, SkBitmap::kARGB_8888_Config, SkImageDecoder::kDecodePixels_Mode, NULL);
	

  //dst_va = (unsigned char*) srcImage.getPixels();

  //memcpy(dst_va,src_va,BUFFER_SIZE);

	//save image
	//saveFile("/data/otis/output.png", srcImage);
	
	//saveFile("/data/otis/output.jpg", *decoded);

  return 0;
}
 

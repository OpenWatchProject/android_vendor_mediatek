/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef __SWSTREAM_H__
#define __SWSTREAM_H__


#ifdef MTK_LOG_ENABLE
#undef MTK_LOG_ENABLE
#endif
#define MTK_LOG_ENABLE 1
#include <stdio.h>
#include <cutils/log.h>
#include <utils/Errors.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>

#include <stdio.h>
#include <cutils/log.h>
#include <utils/Errors.h>
#include <fcntl.h>
#include <sys/mman.h>

//#include "SkTypes.h"
//#include "SkJpegUtility.h"
//
//extern "C" {
//    #include "jpeglib_alpha.h"
//    #include "jerror_alpha.h"
//}
//#include "img_dec_comp.h"

#include "SwType.h"

/*******************************************************************************
*
********************************************************************************/
#ifndef JUINT32
typedef unsigned int JUINT32;
#endif

#ifndef JINT32
typedef int JINT32;
#endif

//#define ALOGD(...) 
 

#include "img_common_def.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
 
//#define JPEG_PROFILING   

#define LOG_TAG "SwStream"

#undef bool
#define bool bool


class SwStream {
public:
    SwStream(){};
    virtual ~SwStream(){};
    /** Called to rewind to the beginning of the stream. If this cannot be
        done, return false.
    */
  	virtual bool setBufStream(void *hw_buffer, size_t hw_buffer_size){
      // unused params
      (void)hw_buffer;
      (void)hw_buffer_size;

  	  return true ;
    }    
    virtual bool rewind() = 0;
    /** If this stream represents a file, this method returns the file's name.
        If it does not, it returns NULL (the default behavior).
    */
    virtual const char* getFileName(){
       // override in subclass if you represent a file
       return NULL;
    }
    /** Called to read or skip size number of bytes.
        If buffer is NULL and size > 0, skip that many bytes, returning how many were skipped.
        If buffer is NULL and size == 0, return the total length of the stream.
        If buffer != NULL, copy the requested number of bytes into buffer, returning how many were copied.
        @param buffer   If buffer is NULL, ignore and just skip size bytes, otherwise copy size bytes into buffer
        @param size The number of bytes to skip or copy
        @return bytes read on success
    */
    virtual size_t read(void* buffer, size_t size) = 0;

    /** Return the total length of the stream.
    */
    size_t getLength() { return this->read(NULL, 0); }

    virtual size_t seek(size_t offset) {
      // unused params
      (void)offset;

      return 0;
    }
    
    /** Skip the specified number of bytes, returning the actual number
        of bytes that could be skipped.
    */
    virtual size_t skip(size_t bytes){return bytes;}

    /** If the stream is backed by RAM, this method returns the starting
        address for the data. If not (i.e. it is backed by a file or other
        structure), this method returns NULL.
        The default implementation returns NULL.
    */
    virtual const void* getMemoryBase(){
      // override in subclass if you represent a memory block
      return NULL;
    }

    int8_t   readS8();
    int16_t  readS16();
    int32_t  readS32();

    uint8_t  readU8() { return (uint8_t)this->readS8(); }
    uint16_t readU16() { return (uint16_t)this->readS16(); }
    uint32_t readU32() { return (uint32_t)this->readS32(); }

    bool     readBool() { return this->readU8() != 0; }
    //SkScalar readScalar();
    size_t   readPackedUInt();
};


class SwBufStream : public SwStream {

public:


	SwBufStream(void *hw_buffer, size_t hw_buffer_size){
	  //JPG_DBG("SwBufStream::SwBufStream %p, %x!!\n", hw_buffer, hw_buffer_size);
	  hwInputBuf = hw_buffer;
	  hwInputBufSize = hw_buffer_size;
	  total_read_size = 0;
	  fSize = 0;
    }

	virtual ~SwBufStream(){
	  //JPG_DBG("SwBufStream::~SwBufStream!!\n");		
	}


  	virtual bool setBufStream(void *hw_buffer, size_t hw_buffer_size){
  	  //JPG_DBG("SwBufStream::setBufStream %p, %zx!!\n", hw_buffer, hw_buffer_size);
  	  if(total_read_size > hw_buffer_size)
  	    return false ;

  	  hwInputBuf = hw_buffer;
  	  hwInputBufSize = hw_buffer_size;
  	  
  	  return true ;
    }


	
    virtual bool rewind(){
	  //JPG_DBG("SwBufStream::rewind, readSize %x, hwBuffSize %x!!\n",   total_read_size, hwInputBufSize);				
	  if(total_read_size > hwInputBufSize){
		   return false;
	  }else if (total_read_size <= hwInputBufSize){
       total_read_size = 0;
	  }
      return true ;
    }
	
	virtual size_t read(void* buffer, size_t size){
		size_t read_start = total_read_size;
		size_t read_end = total_read_size + size ;
		size_t size_1 ;
		size_t size_2 ;
		size_t real_size_2 ;

		//JPG_DBG("SwBufStream::read, buf %x, size %x, tSize %x, st %x, end %x, HWsize %x!!\n", (unsigned long)buffer, (unsigned int) size
		//	, total_read_size, read_start, read_end, hwInputBufSize);			   

		if (buffer == NULL && size == 0){	// special signature, they want the total size
		   fSize = hwInputBufSize ;//+ srcStream->getLength();
			 return fSize;
		}else if(size == 0){
       return 0;
		}

		// if buffer is NULL, seek ahead by size

    if( read_start <= hwInputBufSize && read_end <= hwInputBufSize){
		  if(buffer){
         memcpy(buffer, (const char*)hwInputBuf + read_start, size);
		  }

		  total_read_size += size ;
		  //JPG_DBG("SwBufStream::read(HW), size %x, total_size %x!!\n", size, total_read_size);			   		  					  			
		  return size ;					  

		}else if ( read_start >= hwInputBufSize  ){
      real_size_2 = 0;//srcStream->read(buffer, size);
		  total_read_size += real_size_2 ;
		  //JPG_DBG("SwBufStream::read(Stream), size %x, size_2 %x, total_size %x!!\n", size, real_size_2, total_read_size);
      return real_size_2;
		}else{
      size_1 = hwInputBufSize - read_start ;
      size_2 =  read_end - hwInputBufSize  ;	
		  if (buffer){
         memcpy(buffer, (const char*)hwInputBuf + read_start, size_1);
		  }
		  total_read_size += size_1 ;
		  real_size_2 = 0;//srcStream->read(buffer, size_2);
		  total_read_size += real_size_2 ;
		  //JPG_DBG("SwBufStream::read(HW+Stream), size %x, size_2 %x, total_size %x!!\n", size_2, real_size_2, total_read_size);  
		  return size_1+ real_size_2 ;		

		}



	}

	size_t seek(size_t offset){
      // unused params
      (void)offset;

	    //JPG_DBG("SwBufStream::seek size %x!!\n", offset);			
		return 0;
	}
	size_t skip(size_t size)
	{
		/*	Check for size == 0, and just return 0. If we passed that
			to read(), it would interpret it as a request for the entire
			size of the stream.
		*/
        //JPG_DBG("SwBufStream::skip %x!!\n", size);					
		return size ? this->read(NULL, size) : 0;
	}




private:	
  size_t total_read_size ;
  //SwStream* srcStream;
  void *hwInputBuf ;
  size_t hwInputBufSize ; 
  size_t fSize;
  



};




class SwEncStream  {
public:
    SwEncStream();
    virtual ~SwEncStream();

    /** Called to write bytes to a SkWStream. Returns true on success
        @param buffer the address of at least size bytes to be written to the stream
        @param size The number of bytes in buffer to write to the stream
        @return true on success
    */
    virtual bool write(const void* buffer, size_t size) = 0;
    virtual void newline();
    virtual void flush();

    // helpers
#if 0    
    bool    write8(U8CPU);
    bool    write16(U16CPU);
    bool    write32(uint32_t);

    bool    writeText(const char text[]);
    bool    writeDecAsText(int32_t);
    bool    writeBigDecAsText(int64_t, int minDigits = 0);
    bool    writeHexAsText(uint32_t, int minDigits = 0);
    bool    writeScalarAsText(SkScalar);
    
    bool    writeBool(bool v) { return this->write8(v); }
    bool    writeScalar(SkScalar);
    bool    writePackedUInt(size_t);
    
    bool writeStream(SwStream* input, size_t length);
#endif    
};




#endif 

#include "camera_custom_nvram.h"
#include "camera_awb_tuning_para_imx318mipiraw.h"

#define NVRAM_TUNING_PARAM_NUM  6511001

template <>
AWB_NVRAM_T const&
getAWBNVRAM_imx318mipiraw<CAM_SCENARIO_VIDEO>() // VIDEO
{
    static AWB_NVRAM_T  g_rAWBNVRAM =
    AWB_NVRAM_START
    {
       {
                // AWB calibration data
                {
                        // rUnitGain (unit gain: 1.0 = 512)
                        {
                                0,    // i4R
                                0,    // i4G
                                0    // i4B
                        },
                        // rGoldenGain (golden sample gain: 1.0 = 512)
                        {
                                0,    // i4R
                                0,    // i4G
                                0    // i4B
                        },
                            // rUnitGain TL84 (unit gain: 1.0 = 512)
                            {
                                0,    // i4R
                                0,    // i4G
                                0     // i4B
                            },
                            // rGoldenGain TL84 (golden sample gain: 1.0 = 512)
                            {
                                0,    // i4R
                                0,    // i4G
                                0     // i4B
                            },
                             // rUnitGain Alight (unit gain: 1.0 = 512)
                            {
                                0,    // i4R
                                0,    // i4G
                                0     // i4B
                            },
                            // rGoldenGain Alight (golden sample gain: 1.0 = 512)
                            {
                                0,    // i4R
                                0,    // i4G
                                0     // i4B
                            },
                        // rTuningUnitGain (Tuning sample unit gain: 1.0 = 512)
                        {
                                0,    // i4R
                                0,    // i4G
                                0    // i4B
                        },
                        // rD65Gain (D65 WB gain: 1.0 = 512)
                        {
                                1095,    // i4R
                                512,    // i4G
                                736    // i4B
                        }
                },
                // Original XY coordinate of AWB light source
                {
                        // Strobe
                        {
                                0,    // i4X
                                0    // i4Y
                        },
                        // Horizon
                        {
		-425,	// i4X
		-449	// i4Y	
                        },
                        // A
                        {
			-278,	// i4X	
			-462	    // i4Y	
                        },
                        // TL84
                        {
			-78,	// i4X		
			-493	// i4Y		
                        },
                        // CWF
                        {
			-37,	// i4X	
			-530	// i4Y	
                        },
                        // DNP
                        {
			23,	// i4X		
			-487	// i4Y		
                        },
                        // D65
                        {
			188,	// i4X			
			-442	// i4Y			
                        },
                        // DF
                        {
			168,    // i4X
			-527	// i4Y	
                        }
                },
                // Rotated XY coordinate of AWB light source
                {
                        // Strobe
                        {
                                0,    // i4X
                                0    // i4Y
                        },
                        // Horizon
                        {
			-441,	// i4X				                  
			-434	// i4Y				                  
                        },
                        // A
                        {
			-294,	// i4X				                    
			-452	// i4Y				                    
                        },
                        // TL84
                        {
			-95,	// i4X				                
			-490	// i4Y				                
                        },
                        // CWF
                        {
			-56,	// i4X				                  
			-529	// i4Y				                  
                        },
                        // DNP
                        {
			6,	// i4X				                  
			-488	// i4Y				                  
                        },
                        // D65
                        {
			172,	// i4X				                    
			-449	// i4Y				                  
                        },
                        // DF
                        {
			149,	// i4X				                    
			-533	// i4Y				                      
                        }
                },
                // AWB gain of AWB light source
                {
                        // Strobe
                        {
                                512,    // i4R
                                512,    // i4G
                                512    // i4B
                        },
                        // Horizon
                        {
			529,	// i4R				            
                                512,    // i4G
			1671	// i4B				            
                        },
                        // A
                        {
			656,	// i4R				              
                                512,    // i4G
			1394	// i4B				              
                        },
                        // TL84
                        {
			898,	// i4R				            
                                512,    // i4G
			1109	// i4B				            
                        },
                        // CWF
                        {
			998,	// i4R				            
                                512,    // i4G
			1103	// i4B				            
                        },
                        // DNP
                        {
			1022,	// i4R				            
                                512,    // i4G
			959	// i4B				              
                        },
                        // D65
                        {
			1202,	// i4R				            
                                512,    // i4G
			722	// i4B				              
                        },
                        // DF
                        {
			1312,	// i4R				              
                                512,    // i4G
			833	// i4B				                
                        }
                },
                // Rotation matrix parameter
                {
		2,	// i4RotationAngle					              
                        256,    // i4Cos
		9	// i4Sin					                        
                },
                // Daylight locus parameter
                {
		-138,	// i4SlopeNumerator		
                        128    // i4SlopeDenominator
                },
              // Predictor gain
                {
                        // i4PrefRatio100
                        101,

                        // DaylightLocus_L
                        {
                            1233,    // i4R
                            512, // i4G
                            703     // i4B
                        },
                        // DaylightLocus_H
                        {
                            938,    // i4R
                            512, // i4G
                            943     // i4B
                        },
                        // Temporal General
                        {
                            1233,    // i4R
                            512, // i4G
                            703     // i4B
                        },
                        // AWB LSC Gain
                        {
                            938,    // i4R
                            512, // i4G
                            943     // i4B
                        }
                },
                // AWB light area
                {
                    // Strobe:FIXME
                        {
                        0,    // i4RightBound
                        0,    // i4LeftBound
                        0,    // i4UpperBound
                        0    // i4LowerBound
                        },
                        // Tungsten
                        {
			-195,	// TungRightBound				            
			-841,	// TungLeftBound				            
			-399,	// TungUpperBound				            
			-471	// TungLowerBound				            
                        },
                        // Warm fluorescent
                        {
			-195,	// WFluoRightBound				          
			-841,	// WFluoLeftBound				            
			-471,	// WFluoUpperBound				          
			-529	// WFluoLowerBound				          
                        },
                        // Fluorescent
                        {
			-19,	// FluoRightBound				            
			-195,	// FluoLeftBound				            
			-417,	// FluoUpperBound				            
			-514	// FluoLowerBound				            
                        },
                        // CWF
                        {
			26,	// CWFRightBound				            
			-170,	// CWFLeftBound				              
			-514,	// CWFUpperBound				            
			-574	// CWFLowerBound				            
                        },
                        // Daylight
                        {
			202,	// DayRightBound				              
			-19,	// DayLeftBound				              
			-417,	// DayUpperBound				            
			-514	// DayLowerBound				            
                        },
                        // Shade
                        {
			532,	// ShadeRightBound				          
			202,	// ShadeLeftBound				              
			-417,	// ShadeUpperBound				          
			-481	// ShadeLowerBound				          
                        },
                        // Daylight Fluorescent
                        {
			202,	// DFRightBound				                
			26,	// DFLeftBound				                
			-514,	// DFUpperBound				                
			-574	// DFLowerBound				                  
                        }
                },
                // PWB light area
                {
                        // Reference area
                        {
			532,	// PRefRightBound				            
			-841,	// PRefLeftBound				            
			-374,	// PRefUpperBound				            
			-574	// PRefLowerBound				            
                        },
                        // Daylight
                        {
			227,	// PDayRightBound				            
			-19,	// PDayLeftBound				            
			-417,	// PDayUpperBound				            
			-514	// PDayLowerBound				            
                        },
                        // Cloudy daylight
                        {
			327,	// PCloudyRightBound				        
			152,	// PCloudyLeftBound				            
			-417,	// PCloudyUpperBound				        
			-514	// PCloudyLowerBound				        
                        },
                        // Shade
                        {
			427,	// PShadeRightBound				          
			152,	// PShadeLeftBound				          
			-417,	// PShadeUpperBound				          
			-514	// PShadeLowerBound				          
                        },
                        // Twilight
                        {
			-19,	// PTwiRightBound				            
			-179,	// PTwiLeftBound				            
			-417,	// PTwiUpperBound				            
			-514	// PTwiLowerBound				            
                        },
                        // Fluorescent
                        {
			222,	// PFluoRightBound				          
			-195,	// PFluoLeftBound				            
			-399,	// PFluoUpperBound				          
			-579	// PFluoLowerBound				          
                        },
                        // Warm fluorescent
                        {
			-194,	// PWFluoRightBound				          
			-394,	// PWFluoLeftBound				          
			-399,	// PWFluoUpperBound				          
			-579	// PWFluoLowerBound				          
                        },
                        // Incandescent
                        {
			-194,	// PIncaRightBound				          
			-394,	// PIncaLeftBound				            
			-417,	// PIncaUpperBound				          
			-514	// PIncaLowerBound				          
                        },
                        // Gray World
                        {
                        5000, // PGWRightBound
                        -5000,  // PGWLeftBound
                        5000, // PGWUpperBound
                        -5000 // PGWLowerBound
                        }
                },
                // PWB default gain
                {
                        // Daylight
                        {
			1125,	// PWB_Day_R				                
      512,  // PWB_Day_G
			812	// PWB_Day_B				                  
                        },
                        // Cloudy daylight
                        {
			1343,	// PWB_Cloudy_R				              
      512,  // PWB_Cloudy_G
			672	// PWB_Cloudy_B				                
                        },
                        // Shade
                        {
			1433,	// PWB_Shade_R				              
      512,  // PWB_Shade_G
			627	// PWB_Shade_B				                
                        },
                        // Twilight
                        {
			863,	// PWB_Twi_R				                
      512,  // PWB_Twi_G
			1079	// PWB_Twi_B				                
                        },
                        // Fluorescent
                        {
			1033,	// PWB_Fluo_R				                
      512,  // PWB_Fluo_G
			951	// PWB_Fluo_B				                  
                        },
                        // Warm fluorescent
                        {
			692,	// PWB_WFluo_R				              
      512,  // PWB_WFluo_G
			1462	// PWB_WFluo_B				              
                        },
                        // Incandescent
                        {
			669,	// PWB_Inca_R				                
      512,  // PWB_Inca_G
			1418	// PWB_Inca_B				                
                        },
                        // Gray World
                        {
                        512,  // PWB_GW_R
                        512,  // PWB_GW_G
                        512 // PWB_GW_B
                        }
                },
                // AWB preference color
                {
                        // Tungsten
                        {
			100,	// TUNG_SLIDER				                
			4004	// TUNG_OFFS				                
                        },
                        // Warm fluorescent
                        {
			100,	// WFluo_SLIDER				                
			4004	// WFluo_OFFS				                
                        },
                        // Shade
                        {
      50, // Shade_SLIDER
      909 // Shade_OFFS
                        },
            // Sunset Area
                                {
                                19,   // i4Sunset_BoundXr_Thr
                                -488     // i4Sunset_BoundYr_Thr
                            },
                                // Shade F Area
                                {
                                -70,   // i4BoundXrThr
                                -449    // i4BoundYrThr
                            },
                                // Shade F Vertex
                                {
                                -70,   // i4BoundXrThr
                                -449    // i4BoundYrThr
                            },
                                // Shade CWF Area
                                {
                                -56,   // i4BoundXrThr
                                -529    // i4BoundYrThr
                            },
                                // Shade CWF Vertex
                                {
                                -56,   // i4BoundXrThr
                                -529    // i4BoundYrThr
                            }
                },
// CCT estimation
                {
                        // CCT
                        {
                      2300, // i4CCT[0]
                      2850, // i4CCT[1]
                      3750, // i4CCT[2]
                      5100, // i4CCT[3]
                      6500  // i4CCT[4]
                },
                    // Rotated X coordinate
          {
			                -547,	// i4RotatedXCoordinate[0]
			                -423,	// i4RotatedXCoordinate[1]
			                -271,	// i4RotatedXCoordinate[2]
			                -104,	// i4RotatedXCoordinate[3]
                      0       // i4RotatedXCoordinate[4]
                }
              }
            },

                // Algorithm Tuning Paramter
                {
                    // AWB Backup Enable
                    0,

                     // Daylight locus offset LUTs for tungsten
                {
                    21, // i4Size: LUT dimension
                    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                    {0, 350,  800, 1222, 1444, 1667, 1889, 2111, 2333, 2556, 2778, 3000, 3222, 3444, 3667, 3889, 4111, 4333, 4556, 4778,  5000} // i4LUTOut
                },

                // Daylight locus offset LUTs for WF
                {
                    21, // i4Size: LUT dimension
                    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                    {0, 350,  700,  850, 1200, 1667, 1889, 2111, 2333, 2556, 2778, 3000, 3222, 3444, 3667, 3889, 4111, 4333, 4556, 4778,  5000} // i4LUTOut
                },

                // Daylight locus offset LUTs for shade
                {
                    21, // i4Size: LUT dimension
                    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                    {0, 500, 1000, 1500, 2500, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8000, 8000, 8500, 9000, 9500, 10000}  // i4LUTOut
                },
                // Preference gain for each light source
                {
                    //        LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                    {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                    //        LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                  }, // STROBE
                  {
          	            {530, 530, 480}, {530, 530, 480}, {530, 530, 480}, {530, 530, 480}, {530, 530, 480}, {520, 520, 496}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                  }, // TUNGSTEN
                  {
                        {512, 512, 508}, {512, 512, 508}, {512, 512, 508}, {512, 512, 508}, {512, 512, 508}, {512, 512, 508}, {512, 512, 508}, {512, 512, 508}, {512, 512, 508}, {512, 512, 508},
           	            {512, 512, 508}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                  }, // WARM F
                  {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}
                    }, // F
                    {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    }, // CWF
                    {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}
                    }, // DAYLIGHT
                    {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    }, // SHADE
                    {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    } // DAYLIGHT F
                },
                    // Parent block weight parameter
                    {
                        1,      // bEnable
                        6           // i4ScalingFactor: [6] 1~12, [7] 1~6, [8] 1~3, [9] 1~2, [>=10]: 1
                    },
                    // AWB LV threshold for predictor
                    {
                            100,    // i4InitLVThr_L
                            140,    // i4InitLVThr_H
                            80      // i4EnqueueLVThr
                    },
                    // AWB number threshold for temporal predictor
                    {
                            65,     // i4Neutral_ParentBlk_Thr
                        //LV0   1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
                        { 100, 100, 100, 100, 100, 100, 100, 100, 50,  25,   2,   2,   2,   2,   2,   2,   2,   2,   2}  // (%) i4CWFDF_LUTThr
                    },
                    // AWB light neutral noise reduction for outdoor
                    {
                        //LV0  1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
                        // Non neutral
		                { 5,   5,   5,   5,   5,   5,   5,   5,    5,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
                    // Flurescent
		                { 0,   0,   0,   0,   0,   0,   0,   0,    0,   5,  10,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
                    // CWF
		                { 0,   0,   0,   0,   0,   0,   0,   0,    0,   5,  10,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
                    // Daylight
		                { 0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   2,   4,   4,   4,   4,   4,   4,   4,   4},  // (%)
                    // DF
		                { 0,   0,   0,   0,   0,   0,   0,   0,    0,   5,  10,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
                    },
                    // AWB feature detection
                    {
                        // Sunset Prop
                        {
                            1,          // i4Enable
                            120,        // i4LVThr_L
                          130,        // i4LVThr_H
                            10,         // i4SunsetCountThr
                            0,          // i4SunsetCountRatio_L
                            171         // i4SunsetCountRatio_H
                        },
                        // Shade F Detection
                        {
                            1,          // i4Enable
                            90,        // i4LVThr
                        	105,        // i4LVThr                            
                            128         // i4DaylightProb
                        },
                        // Shade CWF Detection
                        {
                            1,          // i4Enable
                            90,        // i4LVThr
                        	105,        // i4LVThr                            
                            128         // i4DaylightProb
                        }
                    },
                    // AWB non-neutral probability for spatial and temporal weighting look-up table (Max: 100; Min: 0)
                    {
                        //LV0   1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
                        { 100, 100, 100, 100, 100, 100, 100, 100, 100, 70,  30,  20,  10,   0,   0,   0,   0,   0,   0}
                    },

                    // AWB daylight locus probability look-up table (Max: 100; Min: 0)
                    {   //LV0    1     2     3      4     5     6     7     8      9      10     11    12   13     14    15   16    17    18
		                { 100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  50,  0,  0,  0,  0,  0,  0,  0}, // Strobe
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 75,  25,  0,   0,  0,   0,   0}, // Tungsten
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 75,  25,  0,   0,  0,   0,   0}, // Warm fluorescent
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 50,  25,  0,   0,  0,   0,   0}, // Fluorescent
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  50,  25,  0,   0,   0,  0,   0,   0}, // CWF
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 75,  50,  50,  40, 30,  0,   0}, // Daylight
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  50,  25,  0,   0,   0,  0,   0,   0}, // Shade
                        {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  50,  25,  0,   0,   0,  0,   0,   0}  // Daylight fluorescent
                },

                // AWB tuning information
                {
                    6735,       // project code
                    5588,       // model
                    20150624,   // date
                    0,          // reserved 0
                    1,          // reserved 1
                    2,          // reserved 2
                    3,          // reserved 3
                    4,          // reserved 4
                },
    }
};
    return g_rAWBNVRAM;
};

#ifndef _GLES_WARP_SHADER_H_
#define _GLES_WARP_SHADER_H_

const char gles2_warp_vs[]=
{
	  "#version 300 es \n"
    "precision highp float; \n"

    //"attribute highp vec4 _position; \n"
    //"attribute highp vec2 _texcoord; \n"
    "#define VERTEX_ARRAY 0 \n"
    "#define TEXCOORD_ARRAY 1 \n"
    "layout (location = VERTEX_ARRAY) in highp vec4	_position;\n"
		"layout (location = TEXCOORD_ARRAY) in highp vec2	_texcoord;\n"


    "uniform highp sampler2D warp_x_img; \n"
    "uniform highp sampler2D warp_y_img; \n"
    //"uniform float warp_level; \n"
    "uniform float source_width; \n"
    "uniform float source_height; \n"

    "uniform float ori_source_width; \n"
    "uniform float ori_source_height; \n"
    "uniform float buffer_width; \n"
    "uniform float buffer_height; \n"

    //"varying highp vec2 coord2; \n"
    "out highp vec2 coord2; \n"

    "void main() \n"
    "{ \n"
        "float offset_x, offset_y; \n"
        "float pos_x, pos_y; \n"
        "float tempx, tempy; \n"
        "vec2 sample_coord; \n"
        "vec2 tex_coord; \n"

        "tex_coord = vec2(_texcoord.x, _texcoord.y); \n"

        //"pos_x = texture2D(warp_x_img, tex_coord).r; \n"
        //"pos_y = texture2D(warp_y_img, tex_coord).r; \n"
        "pos_x = texture(warp_x_img, tex_coord).r; \n"
        "pos_y = texture(warp_y_img, tex_coord).r; \n"        
        
        "float c1 = (ori_source_width/buffer_width)*(pos_x/source_width); \n"
        "float c2 = (ori_source_height/buffer_height)*(pos_y/source_height); \n"
        "coord2 = vec2(c1, c2 ); \n"
        //"coord2 = vec2(0.0,0.0); \n"
        "gl_Position = _position; \n"
    "} \n"
};

const char gles2_warp_fs[]=
{
    "#extension GL_OES_EGL_image_external : require \n"
    "precision highp float; \n"
    "precision highp int; \n"


    "uniform sampler2D img; \n"
    //"uniform samplerExternalOES  img; \n"
    "varying highp vec2 coord2; \n"

    "void main() \n"
    "{ \n"
        "gl_FragColor= texture2D(img, vec2(coord2.x,coord2.y)); \n"
        //"gl_FragColor= vec4(1.0,0.0,0.0,1.0); \n"
        "gl_FragColor.a = 1.0; \n"
    "} \n"
};

const char gles2_warp_fs_ext[]=
{
	  "#version 300 es \n"
    //"#extension GL_OES_EGL_image_external : require \n"
	  "#extension GL_EXT_YUV_target : require \n"    
    
    "precision highp float; \n"
    "precision highp int; \n"

    "uniform float boundx; \n"
    "uniform float boundy; \n"
    "uniform int yuv_format; \n"

    //"uniform samplerExternalOES  img; \n"
    "uniform __samplerExternal2DY2YEXT   img; \n"
    //"varying highp vec2 coord2; \n"
    "in highp vec2 coord2; \n"
		"layout (yuv) out vec4 oColour0; \n"

    "void main() \n"
    "{ \n"
    " highp vec4 temp_color; \n"
    "    vec2 temp = coord2; \n"
    " temp.x = clamp(temp.x, 0.0, boundx);\n"
    " temp.y = clamp(temp.y, 0.0, boundy);\n"

    //"gl_FragColor= texture2D(img, temp); \n"
    //"gl_FragColor.a = 1.0; \n"
    "temp_color = texture(img, temp); \n"
    "oColour0 = temp_color; \n"
    "switch (yuv_format) \n"
    "{ \n"
    "	case 0: \n"
    "		break; \n"
    "	case 1: \n"
    "		break; \n"
    "	case 2: \n"
    		"oColour0.rgb= yuv_2_rgb (temp_color.rgb, itu_601); \n"
    "		break; \n"
    "	case 3: \n"
    		"oColour0.rgb= yuv_2_rgb (temp_color.rgb, itu_601_full_range); \n"
    "		break; \n"
    "	case 4: \n"
    		"oColour0.rgb= yuv_2_rgb (temp_color.rgb, itu_709); \n"
    "		break; \n"
    "} \n"
    
    "oColour0.a = 1.0; \n"    
    "} \n"
};


#endif
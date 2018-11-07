//#include "stdafx.h"

#ifdef __cplusplus 
extern "C" { 
#endif

#include "libavcodec/avcodec.h"
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>

#include <stdlib.h> 
#include <stdio.h> 
#include <string.h> 
#include <math.h>

#ifdef __cplusplus 
} 
#endif

AVFormatContext *i_fmt_ctx; 
AVStream *i_video_stream;
AVStream *i_audio_stream;

AVFormatContext *o_fmt_ctx; 
AVStream *o_video_stream;
AVStream *o_audio_stream;

int video_index;
int audio_index;


int main(int argc, char **argv) 
{ 
    avcodec_register_all(); 
    av_register_all(); 
    avformat_network_init();

    /* should set to NULL so that avformat_open_input() allocate a new one */ 
    i_fmt_ctx = NULL; 
    //char rtspUrl[] = "rtsp://admin:12345@192.168.10.76:554"; 
    char rtspUrl[] = "rtmp://58.200.131.2:1935/livetv/hunantv"; 
    const char *filename = "rtmp1.mp4"; 
    if (avformat_open_input(&i_fmt_ctx, rtspUrl, NULL, NULL)!=0) 
    { 
        fprintf(stderr, "could not open input file\n"); 
        return -1; 
    }

    if (avformat_find_stream_info(i_fmt_ctx, NULL)<0) 
    { 
        fprintf(stderr, "could not find stream info\n"); 
        return -1; 
    }

    //av_dump_format(i_fmt_ctx, 0, argv[1], 0);

    /* find first video stream */ 
    for (unsigned i=0; i<i_fmt_ctx->nb_streams; i++) 
    { 
        if (i_fmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) 
        { 
            i_video_stream = i_fmt_ctx->streams[i]; 
			video_index = i;
            break; 
        } 
    } 
    if (i_video_stream == NULL) 
    { 
        fprintf(stderr, "didn't find any video stream\n"); 
        return -1; 
    }

	/* find first audio stream */ 
    for (unsigned j = 0; j<i_fmt_ctx->nb_streams; j++) 
    { 
        if (i_fmt_ctx->streams[j]->codec->codec_type == AVMEDIA_TYPE_AUDIO) 
        { 
            i_audio_stream = i_fmt_ctx->streams[j]; 
			audio_index = j;
            break; 
        } 
    } 
    if (i_audio_stream == NULL) 
    { 
        fprintf(stderr, "didn't find any audio stream\n"); 
        return -1; 
    }
	printf("video, audio: %d, %d.\n", video_index, audio_index);

    avformat_alloc_output_context2(&o_fmt_ctx, NULL, NULL, filename);

    /* 
    * since all input files are supposed to be identical (framerate, dimension, color format, ...) 
    * we can safely set output codec values from first input file 
    */ 
    for(int ii = 0; ii < i_fmt_ctx->nb_streams; ii++)
    {
        if(ii == video_index){
		    o_video_stream = avformat_new_stream(o_fmt_ctx, NULL); 
		    if(1){ 
		        AVCodecContext *c; 
		        c = o_video_stream->codec; 
		        c->bit_rate = 400000; 
		        c->codec_id = i_video_stream->codec->codec_id; 
		        c->codec_type = i_video_stream->codec->codec_type; 
		        c->time_base.num = i_video_stream->time_base.num; 
		        c->time_base.den = i_video_stream->time_base.den; 
		        fprintf(stderr, "time_base.num = %d time_base.den = %d\n", c->time_base.num, c->time_base.den); 
		        c->width = i_video_stream->codec->width; 
		        c->height = i_video_stream->codec->height; 
		        c->pix_fmt = i_video_stream->codec->pix_fmt; 
		        fprintf(stderr, "%d %d %d.\n", c->width, c->height, c->pix_fmt); 
		        c->flags = i_video_stream->codec->flags; 
		        c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER; 
		        c->me_range = i_video_stream->codec->me_range; 
		        c->max_qdiff = i_video_stream->codec->max_qdiff;

		        c->qmin = i_video_stream->codec->qmin; 
		        c->qmax = i_video_stream->codec->qmax;

		        c->qcompress = i_video_stream->codec->qcompress; 

				o_video_stream->time_base.num = c->time_base.num;
			    o_video_stream->time_base.den = c->time_base.den;
				if (i_video_stream ->codec->extradata) {
                       c->extradata = (uint8_t *)av_mallocz(i_video_stream ->codec->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
                       
                       memcpy(c->extradata, i_video_stream ->codec->extradata, i_video_stream ->codec->extradata_size);
                       c->extradata_size = i_video_stream ->codec->extradata_size;
				}
		    }
        }
		if(ii == audio_index){
			o_audio_stream = avformat_new_stream(o_fmt_ctx, NULL); 
		    if(1){ 
		        AVCodecContext *c; 
		        c = o_audio_stream->codec; 
		        //c->bit_rate = 400000; 
				c->bit_rate           =  i_audio_stream ->codec->bit_rate;   
				c->gop_size           =  i_audio_stream ->codec->gop_size;
				c->channel_layout =  i_audio_stream ->codec->channel_layout;  
				c->frame_size     =  i_audio_stream ->codec->frame_size;
				c->sample_rate        =  i_audio_stream ->codec->sample_rate; 
				c->channels           =  i_audio_stream ->codec->channels;
				c->block_align        =  i_audio_stream ->codec->block_align; 
				
		        c->codec_id = i_audio_stream->codec->codec_id; 
		        c->codec_type = i_audio_stream->codec->codec_type; 
		        c->time_base.num = i_audio_stream->time_base.num; 
		        c->time_base.den = i_audio_stream->time_base.den; 
		        fprintf(stderr, "time_base.num = %d time_base.den = %d\n", c->time_base.num, c->time_base.den); 
		        
		        c->pix_fmt = i_audio_stream->codec->pix_fmt; 
		        c->flags = i_audio_stream->codec->flags; 
		        c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER; 
		        //c->me_range = i_audio_stream->codec->me_range; 
		        //c->max_qdiff = i_audio_stream->codec->max_qdiff;

		        //c->qmin = i_audio_stream->codec->qmin; 
		        //c->qmax = i_audio_stream->codec->qmax;

		        c->qcompress = i_audio_stream->codec->qcompress; 
				o_audio_stream->time_base.num = c->time_base.num;
			    o_audio_stream->time_base.den = c->time_base.den;
                if (i_audio_stream ->codec->extradata) {
                    c->extradata = (uint8_t *)av_mallocz(i_audio_stream ->codec->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
                             
                    memcpy(c->extradata, i_audio_stream ->codec->extradata, i_audio_stream ->codec->extradata_size);
                    c->extradata_size = i_audio_stream ->codec->extradata_size;
                }
		    }
			
		}
    }
    avio_open(&o_fmt_ctx->pb, filename, AVIO_FLAG_WRITE);

    avformat_write_header(o_fmt_ctx, NULL);

    int last_pts = 0; 
    int last_dts = 0;

    int64_t pts, dts; 
	int total_nul = 200;
    while (1) 
    { 
        AVPacket i_pkt; 
        av_init_packet(&i_pkt); 
        i_pkt.size = 0; 
        i_pkt.data = NULL; 
        if (av_read_frame(i_fmt_ctx, &i_pkt) <0 ) 
            break; 
		printf("index: %lld\n", i_pkt.stream_index); 
		if(i_pkt.stream_index == audio_index)
		{
		 //   continue;
		}
        printf("read: %lld %lld\n", i_pkt.pts, i_pkt.dts); 
		i_pkt.pts = i_pkt.pts*1000 - i_fmt_ctx->start_time;
        i_pkt.dts = i_pkt.dts*1000 - i_fmt_ctx->start_time;

        /* 
        * pts and dts should increase monotonically 
        * pts should be >= dts 
        */ 
        //i_pkt.flags |= AV_PKT_FLAG_KEY; 
		if(i_pkt.stream_index == audio_index){
        	i_pkt.pts = i_pkt.pts * 48/ 1000; 
			i_pkt.dts = i_pkt.dts * 48/ 1000; 
    	}
		else if(i_pkt.stream_index == video_index){
        	i_pkt.pts = i_pkt.pts * 16/1000; 
			i_pkt.dts = i_pkt.dts * 16/1000; 
    	}
    	
		pts = i_pkt.pts;
        i_pkt.pts += last_pts; 
        dts = i_pkt.dts; 
        i_pkt.dts += last_dts; 
        //i_pkt.stream_index = 0;

        //printf("%lld %lld\n", i_pkt.pts, i_pkt.dts); 
        static int num = 1; 
        printf("frame %d\n", num++); 
		printf("write: %lld %lld\n", i_pkt.pts, i_pkt.dts); 
        av_interleaved_write_frame(o_fmt_ctx, &i_pkt); 
		if(num >= total_nul){
		    break;
		}
        //av_free_packet(&i_pkt); 
        //av_init_packet(&i_pkt); 
    } 
    last_dts += dts; 
    last_pts += pts;

    avformat_close_input(&i_fmt_ctx);

    av_write_trailer(o_fmt_ctx);

    avcodec_close(o_fmt_ctx->streams[0]->codec); 
    av_freep(&o_fmt_ctx->streams[0]->codec); 
    av_freep(&o_fmt_ctx->streams[0]);

    avio_close(o_fmt_ctx->pb); 
    av_free(o_fmt_ctx);

    return 0; 
}

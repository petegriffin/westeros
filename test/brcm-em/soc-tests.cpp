/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2018 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/input.h>
#include <xkbcommon/xkbcommon.h>

#include <glib.h>
#include <gst/gst.h>

#include "../westeros-ut-em.h"
#include "../test-egl.h"
#include "soc-tests.h"
#include "soc-video-src.h"

#include "wayland-client.h"
#include "wayland-egl.h"

#include "simpleshell-client-protocol.h"

#include "bmedia_types.h"
#include "nexus_config.h"
#include "nexus_platform.h"

#include "westeros-gl.h"
#include "westeros-compositor.h"
#include "westeros-render.h"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define INTERVAL_200_MS (200000)

static bool testCaseSocAPIRendererModule( EMCTX *emctx );
static bool testCaseSocSimpleShellBasic( EMCTX *emctx );
static bool testCaseSocAPISetDefaultCursor( EMCTX *emctx );
static bool testCaseSocSinkInit( EMCTX *emctx );
static bool testCaseSocSinkBasicPipeline( EMCTX *ctx );
static bool testCaseSocSinkFirstFrameSignal( EMCTX *ctx );
static bool testCaseSocSinkUnderflowSignal( EMCTX *ctx );
static bool testCaseSocSinkPtsErrorSignal( EMCTX *ctx );
static bool testCaseSocSinkBasicPositionReporting( EMCTX *ctx );
static bool testCaseSocSinkBasicPauseResume( EMCTX *ctx );
static bool testCaseSocSinkBasicSeek( EMCTX *ctx );
static bool testCaseSocSinkBasicSeekZeroBased( EMCTX *ctx );
static bool testCaseSocSinkServerPlaySpeedDecodeRate( EMCTX *ctx );
static bool testCaseSocSinkInitWithCompositor( EMCTX *emctx );
static bool testCaseSocSinkBasicPipelineWithCompositor( EMCTX *ctx );
static bool testCaseSocSinkFirstFrameSignalWithCompositor( EMCTX *ctx );
static bool testCaseSocSinkUnderflowSignalWithCompositor( EMCTX *ctx );
static bool testCaseSocSinkPtsErrorSignalWithCompositor( EMCTX *ctx );
static bool testCaseSocSinkBasicPositionReportingWithCompositor( EMCTX *ctx );
static bool testCaseSocSinkBasicPauseResumeWithCompositor( EMCTX *ctx );
static bool testCaseSocSinkBasicSeekWithCompositor( EMCTX *ctx );
static bool testCaseSocSinkBasicSeekZeroBasedWithCompositor( EMCTX *ctx );
static bool testCaseSocSinkBasicPipelineGfx( EMCTX *ctx );
static bool testCaseSocSinkVP9NonHDR( EMCTX *emctx );
static bool testCaseSocSinkVP9HDRColorParameters( EMCTX *emctx );
static bool testCaseSocSinkGfxTransition( EMCTX *emctx );

TESTCASE socTests[]=
{
   { "testSocAPIRendererModule",
     "Test compositor renderer module API paths with nexus render module",
     testCaseSocAPIRendererModule
   },
   { "testSocSimpleShellBasic",
     "Test simple shell paths with nexus render module",
     testCaseSocSimpleShellBasic
   },
   { "testSocAPISetDefaultCursor",
     "Test compositor set default cursor API paths with nexus render module",
     testCaseSocAPISetDefaultCursor
   },
   { "testSocSinkInit",
     "Test loading westerossink",
     testCaseSocSinkInit
   },
   { "testSocSinkBasicPipeline",
     "Test creating a basic pipeline with westerossink",
     testCaseSocSinkBasicPipeline
   },
   { "testSocSinkFirstFrameSignal",
     "Test first frame signal",
     testCaseSocSinkFirstFrameSignal
   },
   { "testSocSinkUnderflowSignal",
     "Test underflow signal",
     testCaseSocSinkUnderflowSignal
   },
   { "testSocSinkPtsErrorSignal",
     "Test pts error signal",
     testCaseSocSinkPtsErrorSignal
   },
   { "testSocSinkBasicPositionReporting",
     "Test basic position reporting from a pipeline",
     testCaseSocSinkBasicPositionReporting
   },
   { "testSocSinkBasicPauseResume",
     "Test basic pause and resume",
     testCaseSocSinkBasicPauseResume
   },
   { "testSocSinkBasicSeek",
     "Test basic seek operation",
     testCaseSocSinkBasicSeek
   },
   { "testSocSinkBasicSeekZeroBased",
     "Test basic seek operation with zero based segments",
     testCaseSocSinkBasicSeekZeroBased
   },
   { "testSocSinkServerPlaySpeedDecodeRate",
     "Test decode rate with server play speed property",
     testCaseSocSinkServerPlaySpeedDecodeRate
   },
   { "testSocSinkInitWithCompositor",
     "Test loading westerossink with a compositor",
     testCaseSocSinkInitWithCompositor
   },
   { "testSocSinkBasicPipelineWithCompositor",
     "Test creating a basic pipeline with westerossink with a compositor",
     testCaseSocSinkBasicPipelineWithCompositor
   },
   { "testSocSinkFirstFrameSignalWithCompositor",
     "Test first frame signal with a compositor",
     testCaseSocSinkFirstFrameSignalWithCompositor
   },
   { "testSocSinkUnderflowSignalWithCompositor",
     "Test underflow signal with a compositor",
     testCaseSocSinkUnderflowSignalWithCompositor
   },
   { "testSocSinkPtsErrorSignalWithCompositor",
     "Test pts error signal with a compositor",
     testCaseSocSinkPtsErrorSignalWithCompositor
   },
   { "testSocSinkBasicPositionReportingWithCompositor",
     "Test basic position reporting from a pipeline with a compositor",
     testCaseSocSinkBasicPositionReportingWithCompositor
   },
   { "testSocSinkBasicPauseResumeWithCompositor",
     "Test basic pause and resume with a compositor",
     testCaseSocSinkBasicPauseResumeWithCompositor
   },
   { "testSocSinkBasicSeekWithCompositor",
     "Test basic seek operation with a compositor",
     testCaseSocSinkBasicSeekWithCompositor
   },
   { "testSocSinkBasicSeekZeroBasedWithCompositor",
     "Test basic seek operation with zero based segments with a compositor",
     testCaseSocSinkBasicSeekZeroBasedWithCompositor
   },
   { "testSocSinkBasicPipelineGfx",
     "Test creating a basic pipeline with westerossink using graphics path",
     testCaseSocSinkBasicPipelineGfx
   },
   { "testSocSinkVP9NonHDR",
     "Test handling non-HDR VP9 with westerossink",
     testCaseSocSinkVP9NonHDR
   },
   { "testSocSinkVP9HDRColorParameters",
     "Test handling VP9 HDR color parameters with westerossink",
     testCaseSocSinkVP9HDRColorParameters
   },
   { "testSocSinkGfxTransition",
     "Test westerossink transition from HW to graphics path",
     testCaseSocSinkGfxTransition
   },
   {
     "", "", (TESTCASEFUNC)0
   }
};

TESTCASE getSocTest( int index )
{
   return socTests[index];
}

static gint64 getSegmentStart( EMSimpleVideoDecoder *dec, gint64 time )
{
   bool startAtZero= EMSimpleVideoDecoderGetSegmentsStartAtZero( dec );

   if ( startAtZero )
   {
      return 0;
   }
   else
   {
      return time;
   }
}

static bool testCaseSocAPIRendererModule( EMCTX *emctx )
{
   bool testResult= false;
   bool result;
   WstCompositor *wctx= 0;
   const char *rendererName1= "libwesteros_render_nexus.so.0.0.0";

   wctx= WstCompositorCreate();
   if ( !wctx )
   {
      EMERROR( "WstCompositorCreate failed" );
      goto exit;
   }

   result= WstCompositorSetRendererModule( wctx, rendererName1 );
   if ( result == false )
   {
      EMERROR( "WstCompositorSetRendererModule failed" );
      goto exit;
   }

   result= WstCompositorStart( wctx );
   if ( result == false )
   {
      EMERROR( "WstCompositorStart failed" );
      goto exit;
   }

   WstCompositorDestroy( wctx );

   testResult= true;

exit:

   return testResult;
}

static bool testCaseSocSinkInit( EMCTX *emctx )
{
   bool testResult= false;
   int argc= 0;
   char **argv= 0;
   GstElement *sink= 0;

   gst_init( &argc, &argv );

   sink= gst_element_factory_make( "westerossink", "vsink" );
   if ( !sink )
   {
      EMERROR("Failed to create sink instance");
      goto exit;
   }

   gst_element_set_state( sink, GST_STATE_PAUSED );

   gst_element_set_state( sink, GST_STATE_NULL );

   testResult= true;

exit:
   if ( sink )
   {
      gst_object_unref( sink );
   }

   return testResult;
}

static bool testCaseSocSinkBasicPipeline( EMCTX *emctx )
{
   bool testResult= false;
   int argc= 0;
   char **argv= 0;
   GstElement *pipeline= 0;
   GstElement *src= 0;
   GstElement *sink= 0;
   EMSimpleVideoDecoder *videoDecoder= 0;
   int stcChannelProxy;
   int videoPidChannelProxy;

   videoDecoder= EMGetSimpleVideoDecoder( emctx, EM_TUNERID_MAIN );
   if ( !videoDecoder )
   {
      EMERROR("Failed to obtain test video decoder");
      goto exit;
   }

   EMSetStcChannel( emctx, (void*)&stcChannelProxy );
   EMSetVideoCodec( emctx, bvideo_codec_h264 );
   EMSetVideoPidChannel( emctx, (void*)&videoPidChannelProxy );
   EMSimpleVideoDecoderSetVideoSize( videoDecoder, 1920, 1080 );

   gst_init( &argc, &argv );

   pipeline= gst_pipeline_new("pipeline");
   if ( !pipeline )
   {
      EMERROR("Failed to create pipeline instance");
      goto exit;
   }

   src= createVideoSrc( emctx, videoDecoder );
   if ( !src )
   {
      EMERROR("Failed to create src instance");
      goto exit;
   }

   sink= gst_element_factory_make( "westerossink", "vsink" );
   if ( !sink )
   {
      EMERROR("Failed to create sink instance");
      goto exit;
   }

   gst_bin_add_many( GST_BIN(pipeline), src, sink, NULL );

   if ( gst_element_link( src, sink ) != TRUE )
   {
      EMERROR("Failed to link src and sink");
      goto exit;
   }

   gst_element_set_state( pipeline, GST_STATE_PLAYING );

   // Allow pipeline to run briefly
   usleep( 2000000 );

   gst_element_set_state( pipeline, GST_STATE_NULL );

   testResult= true;

exit:
   if ( pipeline )
   {
      gst_object_unref( pipeline );
   }

   return testResult;
}

static void firstFrameCallback(GstElement *sink, guint size, void *context, gpointer data)
{
   bool *gotSignal= (bool*)data;

   g_print("received first frame signal\n");
   *gotSignal= true;
}

static bool testCaseSocSinkFirstFrameSignal( EMCTX *emctx )
{
   bool testResult= false;
   int argc= 0;
   char **argv= 0;
   GstElement *pipeline= 0;
   GstElement *src= 0;
   GstElement *sink= 0;
   EMSimpleVideoDecoder *videoDecoder= 0;
   int stcChannelProxy;
   int videoPidChannelProxy;
   bool receivedSignal;

   videoDecoder= EMGetSimpleVideoDecoder( emctx, EM_TUNERID_MAIN );
   if ( !videoDecoder )
   {
      EMERROR("Failed to obtain test video decoder");
      goto exit;
   }

   EMSetStcChannel( emctx, (void*)&stcChannelProxy );
   EMSetVideoCodec( emctx, bvideo_codec_h264 );
   EMSetVideoPidChannel( emctx, (void*)&videoPidChannelProxy );
   EMSimpleVideoDecoderSetVideoSize( videoDecoder, 1920, 1080 );

   gst_init( &argc, &argv );

   pipeline= gst_pipeline_new("pipeline");
   if ( !pipeline )
   {
      EMERROR("Failed to create pipeline instance");
      goto exit;
   }

   src= createVideoSrc( emctx, videoDecoder );
   if ( !src )
   {
      EMERROR("Failed to create src instance");
      goto exit;
   }

   sink= gst_element_factory_make( "westerossink", "vsink" );
   if ( !sink )
   {
      EMERROR("Failed to create sink instance");
      goto exit;
   }

   gst_bin_add_many( GST_BIN(pipeline), src, sink, NULL );

   if ( gst_element_link( src, sink ) != TRUE )
   {
      EMERROR("Failed to link src and sink");
      goto exit;
   }

   g_signal_connect( sink, "first-video-frame-callback", G_CALLBACK(firstFrameCallback), &receivedSignal);

   receivedSignal= false;

   gst_element_set_state( pipeline, GST_STATE_PLAYING );

   // Allow pipeline to run briefly
   usleep( 200000 );

   gst_element_set_state( pipeline, GST_STATE_NULL );

   if ( !receivedSignal )
   {
      EMERROR("Failed to receive first video frame signal");
      goto exit;
   }

   testResult= true;

exit:
   if ( pipeline )
   {
      gst_object_unref( pipeline );
   }

   return testResult;
}

static void underflowCallback(GstElement *sink, guint size, void *context, gpointer data)
{
   bool *gotSignal= (bool*)data;

   g_print("received underflow signal\n");
   *gotSignal= true;
}

static bool testCaseSocSinkUnderflowSignal( EMCTX *emctx )
{
   bool testResult= false;
   int argc= 0;
   char **argv= 0;
   GstElement *pipeline= 0;
   GstElement *src= 0;
   GstElement *sink= 0;
   EMSimpleVideoDecoder *videoDecoder= 0;
   int stcChannelProxy;
   int videoPidChannelProxy;
   bool receivedSignal;

   videoDecoder= EMGetSimpleVideoDecoder( emctx, EM_TUNERID_MAIN );
   if ( !videoDecoder )
   {
      EMERROR("Failed to obtain test video decoder");
      goto exit;
   }

   EMSetStcChannel( emctx, (void*)&stcChannelProxy );
   EMSetVideoCodec( emctx, bvideo_codec_h264 );
   EMSetVideoPidChannel( emctx, (void*)&videoPidChannelProxy );
   EMSimpleVideoDecoderSetVideoSize( videoDecoder, 1920, 1080 );

   gst_init( &argc, &argv );

   pipeline= gst_pipeline_new("pipeline");
   if ( !pipeline )
   {
      EMERROR("Failed to create pipeline instance");
      goto exit;
   }

   src= createVideoSrc( emctx, videoDecoder );
   if ( !src )
   {
      EMERROR("Failed to create src instance");
      goto exit;
   }

   sink= gst_element_factory_make( "westerossink", "vsink" );
   if ( !sink )
   {
      EMERROR("Failed to create sink instance");
      goto exit;
   }

   gst_bin_add_many( GST_BIN(pipeline), src, sink, NULL );

   if ( gst_element_link( src, sink ) != TRUE )
   {
      EMERROR("Failed to link src and sink");
      goto exit;
   }

   g_signal_connect( sink, "buffer-underflow-callback", G_CALLBACK(underflowCallback), &receivedSignal);

   receivedSignal= false;

   gst_element_set_state( pipeline, GST_STATE_PLAYING );

   // Allow pipeline to run briefly
   usleep( 200000 );

   EMSimpleVideoDecoderSignalUnderflow( videoDecoder );

   gst_element_set_state( pipeline, GST_STATE_NULL );

   if ( !receivedSignal )
   {
      EMERROR("Failed to receive underflow signal");
      goto exit;
   }

   testResult= true;

exit:
   if ( pipeline )
   {
      gst_object_unref( pipeline );
   }

   return testResult;
}

static void ptsErrorCallback(GstElement *sink, guint size, void *context, gpointer data)
{
   bool *gotSignal= (bool*)data;

   g_print("received pts error signal\n");
   *gotSignal= true;
}

static bool testCaseSocSinkPtsErrorSignal( EMCTX *emctx )
{
   bool testResult= false;
   int argc= 0;
   char **argv= 0;
   GstElement *pipeline= 0;
   GstElement *src= 0;
   GstElement *sink= 0;
   EMSimpleVideoDecoder *videoDecoder= 0;
   int stcChannelProxy;
   int videoPidChannelProxy;
   bool receivedSignal;

   videoDecoder= EMGetSimpleVideoDecoder( emctx, EM_TUNERID_MAIN );
   if ( !videoDecoder )
   {
      EMERROR("Failed to obtain test video decoder");
      goto exit;
   }

   EMSetStcChannel( emctx, (void*)&stcChannelProxy );
   EMSetVideoCodec( emctx, bvideo_codec_h264 );
   EMSetVideoPidChannel( emctx, (void*)&videoPidChannelProxy );
   EMSimpleVideoDecoderSetVideoSize( videoDecoder, 1920, 1080 );

   gst_init( &argc, &argv );

   pipeline= gst_pipeline_new("pipeline");
   if ( !pipeline )
   {
      EMERROR("Failed to create pipeline instance");
      goto exit;
   }

   src= createVideoSrc( emctx, videoDecoder );
   if ( !src )
   {
      EMERROR("Failed to create src instance");
      goto exit;
   }

   sink= gst_element_factory_make( "westerossink", "vsink" );
   if ( !sink )
   {
      EMERROR("Failed to create sink instance");
      goto exit;
   }

   gst_bin_add_many( GST_BIN(pipeline), src, sink, NULL );

   if ( gst_element_link( src, sink ) != TRUE )
   {
      EMERROR("Failed to link src and sink");
      goto exit;
   }

   g_signal_connect( sink, "pts-error-callback", G_CALLBACK(ptsErrorCallback), &receivedSignal);

   receivedSignal= false;

   gst_element_set_state( pipeline, GST_STATE_PLAYING );

   // Allow pipeline to run briefly
   usleep( 200000 );

   EMSimpleVideoDecoderSignalPtsError( videoDecoder );

   gst_element_set_state( pipeline, GST_STATE_NULL );

   if ( !receivedSignal )
   {
      EMERROR("Failed to receive pts error signal");
      goto exit;
   }

   testResult= true;

exit:
   if ( pipeline )
   {
      gst_object_unref( pipeline );
   }

   return testResult;
}

static bool testCaseSocSinkBasicPositionReporting( EMCTX *emctx )
{
   bool testResult= false;
   int argc= 0;
   char **argv= 0;
   GstElement *pipeline= 0;
   GstElement *src= 0;
   GstElement *sink= 0;
   EMSimpleVideoDecoder *videoDecoder= 0;
   int stcChannelProxy;
   int videoPidChannelProxy;
   float frameRate;
   int frameNumber;
   gint64 pos, posExpected;

   videoDecoder= EMGetSimpleVideoDecoder( emctx, EM_TUNERID_MAIN );
   if ( !videoDecoder )
   {
      EMERROR("Failed to obtain test video decoder");
      goto exit;
   }

   EMSetStcChannel( emctx, (void*)&stcChannelProxy );
   EMSetVideoCodec( emctx, bvideo_codec_h264 );
   EMSetVideoPidChannel( emctx, (void*)&videoPidChannelProxy );
   EMSimpleVideoDecoderSetVideoSize( videoDecoder, 1920, 1080 );

   gst_init( &argc, &argv );

   pipeline= gst_pipeline_new("pipeline");
   if ( !pipeline )
   {
      EMERROR("Failed to create pipeline instance");
      goto exit;
   }

   src= createVideoSrc( emctx, videoDecoder );
   if ( !src )
   {
      EMERROR("Failed to create src instance");
      goto exit;
   }

   sink= gst_element_factory_make( "westerossink", "vsink" );
   if ( !sink )
   {
      EMERROR("Failed to create sink instance");
      goto exit;
   }

   gst_bin_add_many( GST_BIN(pipeline), src, sink, NULL );

   if ( gst_element_link( src, sink ) != TRUE )
   {
      EMERROR("Failed to link src and sink");
      goto exit;
   }

   gst_element_set_state( pipeline, GST_STATE_PLAYING );

   for( int i= 0; i < 10; ++i )
   {
      usleep( INTERVAL_200_MS );

      frameRate= EMSimpleVideoDecoderGetFrameRate( videoDecoder );
      frameNumber= videoSrcGetFrameNumber( src );

      posExpected= (frameNumber/frameRate)*GST_SECOND;
      if ( !gst_element_query_position( pipeline, GST_FORMAT_TIME, &pos ) )
      {
         EMERROR("Failed to query position");
         goto exit;
      }
      g_print("%d position %" GST_TIME_FORMAT " expected %" GST_TIME_FORMAT "\n", i, GST_TIME_ARGS(pos), GST_TIME_ARGS(posExpected));
      if ( (pos < 0.9*posExpected) || (pos > 1.1*posExpected) )
      {
         EMERROR("Position out of range: expected %" GST_TIME_FORMAT " actual %" GST_TIME_FORMAT, GST_TIME_ARGS(posExpected), GST_TIME_ARGS(pos));
         goto exit;
      }
   }

   gst_element_set_state( pipeline, GST_STATE_NULL );

   testResult= true;

exit:
   if ( pipeline )
   {
      gst_object_unref( pipeline );
   }

   return testResult;
}

static bool testCaseSocSinkBasicPauseResume( EMCTX *emctx )
{
   bool testResult= false;
   int argc= 0;
   char **argv= 0;
   GstElement *pipeline= 0;
   GstElement *src= 0;
   GstElement *sink= 0;
   EMSimpleVideoDecoder *videoDecoder= 0;
   int stcChannelProxy;
   int videoPidChannelProxy;
   float frameRate;
   int frameNumber;
   gint64 pos, posExpected;
   bool isPaused;

   videoDecoder= EMGetSimpleVideoDecoder( emctx, EM_TUNERID_MAIN );
   if ( !videoDecoder )
   {
      EMERROR("Failed to obtain test video decoder");
      goto exit;
   }

   EMSetStcChannel( emctx, (void*)&stcChannelProxy );
   EMSetVideoCodec( emctx, bvideo_codec_h264 );
   EMSetVideoPidChannel( emctx, (void*)&videoPidChannelProxy );
   EMSimpleVideoDecoderSetVideoSize( videoDecoder, 1920, 1080 );

   gst_init( &argc, &argv );

   pipeline= gst_pipeline_new("pipeline");
   if ( !pipeline )
   {
      EMERROR("Failed to create pipeline instance");
      goto exit;
   }

   src= createVideoSrc( emctx, videoDecoder );
   if ( !src )
   {
      EMERROR("Failed to create src instance");
      goto exit;
   }

   sink= gst_element_factory_make( "westerossink", "vsink" );
   if ( !sink )
   {
      EMERROR("Failed to create sink instance");
      goto exit;
   }

   gst_bin_add_many( GST_BIN(pipeline), src, sink, NULL );

   if ( gst_element_link( src, sink ) != TRUE )
   {
      EMERROR("Failed to link src and sink");
      goto exit;
   }

   gst_element_set_state( pipeline, GST_STATE_PLAYING );

   isPaused= false;
   for( int i= 0; i < 10; ++i )
   {
      if ( i == 4 )
      {
         gst_element_set_state( pipeline, GST_STATE_PAUSED );
         isPaused= true;
      }
      if ( i == 7 )
      {
         gst_element_set_state( pipeline, GST_STATE_PLAYING );
         isPaused= false;
      }

      usleep( INTERVAL_200_MS );

      frameRate= EMSimpleVideoDecoderGetFrameRate( videoDecoder );
      frameNumber= videoSrcGetFrameNumber( src );

      if ( !isPaused )
      {
         posExpected= (frameNumber/frameRate)*GST_SECOND;
      }

      if ( !gst_element_query_position( pipeline, GST_FORMAT_TIME, &pos ) )
      {
         EMERROR("Failed to query position");
         goto exit;
      }

      g_print("%d paused %d position %" GST_TIME_FORMAT " expected %" GST_TIME_FORMAT "\n", i, isPaused, GST_TIME_ARGS(pos), GST_TIME_ARGS(posExpected));

      if ( (pos < 0.9*posExpected) || (pos > 1.1*posExpected) )
      {
         EMERROR("Position out of range: expected %" GST_TIME_FORMAT " actual %" GST_TIME_FORMAT, GST_TIME_ARGS(posExpected), GST_TIME_ARGS(pos));
         goto exit;
      }
   }

   gst_element_set_state( pipeline, GST_STATE_NULL );

   testResult= true;

exit:
   if ( pipeline )
   {
      gst_object_unref( pipeline );
   }

   return testResult;
}

static bool testCaseSocSinkBasicSeek( EMCTX *emctx )
{
   bool testResult= false;
   int argc= 0;
   char **argv= 0;
   GstElement *pipeline= 0;
   GstElement *src= 0;
   GstElement *sink= 0;
   EMSimpleVideoDecoder *videoDecoder= 0;
   int stcChannelProxy;
   int videoPidChannelProxy;
   float frameRate;
   int frameNumber;
   gint64 pos, posExpected;
   gint64 seekPos;
   gboolean rv;

   videoDecoder= EMGetSimpleVideoDecoder( emctx, EM_TUNERID_MAIN );
   if ( !videoDecoder )
   {
      EMERROR("Failed to obtain test video decoder");
      goto exit;
   }

   EMSetStcChannel( emctx, (void*)&stcChannelProxy );
   EMSetVideoCodec( emctx, bvideo_codec_h264 );
   EMSetVideoPidChannel( emctx, (void*)&videoPidChannelProxy );
   EMSimpleVideoDecoderSetVideoSize( videoDecoder, 1920, 1080 );

   gst_init( &argc, &argv );

   pipeline= gst_pipeline_new("pipeline");
   if ( !pipeline )
   {
      EMERROR("Failed to create pipeline instance");
      goto exit;
   }

   src= createVideoSrc( emctx, videoDecoder );
   if ( !src )
   {
      EMERROR("Failed to create src instance");
      goto exit;
   }

   sink= gst_element_factory_make( "westerossink", "vsink" );
   if ( !sink )
   {
      EMERROR("Failed to create sink instance");
      goto exit;
   }

   gst_bin_add_many( GST_BIN(pipeline), src, sink, NULL );

   if ( gst_element_link( src, sink ) != TRUE )
   {
      EMERROR("Failed to link src and sink");
      goto exit;
   }

   gst_element_set_state( pipeline, GST_STATE_PLAYING );

   for( int i= 0; i < 5; ++i )
   {
      usleep( INTERVAL_200_MS );

      frameRate= EMSimpleVideoDecoderGetFrameRate( videoDecoder );
      frameNumber= videoSrcGetFrameNumber( src );

      posExpected= (frameNumber/frameRate)*GST_SECOND;

      if ( !gst_element_query_position( pipeline, GST_FORMAT_TIME, &pos ) )
      {
         EMERROR("Failed to query position");
         goto exit;
      }

      g_print("%d position %" GST_TIME_FORMAT " expected %" GST_TIME_FORMAT "\n", i, GST_TIME_ARGS(pos), GST_TIME_ARGS(posExpected));

      if ( (pos < 0.9*posExpected) || (pos > 1.1*posExpected) )
      {
         EMERROR("Position out of range: expected %" GST_TIME_FORMAT " actual %" GST_TIME_FORMAT, GST_TIME_ARGS(posExpected), GST_TIME_ARGS(pos));
         goto exit;
      }
   }

   seekPos= 30.0 * GST_SECOND;
   seekPos= getSegmentStart( videoDecoder, seekPos );

   rv= gst_element_seek( pipeline,
                         1.0, //rate
                         GST_FORMAT_TIME,
                         GST_SEEK_FLAG_FLUSH,
                         GST_SEEK_TYPE_SET,
                         seekPos,
                         GST_SEEK_TYPE_NONE,
                         GST_CLOCK_TIME_NONE );
   if ( !rv )
   {
      EMERROR("Seek operation failed");
      goto exit;
   }

   for( int i= 0; i < 5; ++i )
   {
      usleep( INTERVAL_200_MS );

      frameRate= EMSimpleVideoDecoderGetFrameRate( videoDecoder );
      frameNumber= videoSrcGetFrameNumber( src );

      posExpected= seekPos + (frameNumber/frameRate)*GST_SECOND;

      if ( !gst_element_query_position( pipeline, GST_FORMAT_TIME, &pos ) )
      {
         EMERROR("Failed to query position");
         goto exit;
      }

      g_print("%d position %" GST_TIME_FORMAT " expected %" GST_TIME_FORMAT "\n", i, GST_TIME_ARGS(pos), GST_TIME_ARGS(posExpected));

      if ( (pos < 0.9*posExpected) || (pos > 1.1*posExpected) )
      {
         EMERROR("Position out of range: expected %" GST_TIME_FORMAT " actual %" GST_TIME_FORMAT, GST_TIME_ARGS(posExpected), GST_TIME_ARGS(pos));
         goto exit;
      }
   }

   gst_element_set_state( pipeline, GST_STATE_NULL );

   testResult= true;

exit:
   if ( pipeline )
   {
      gst_object_unref( pipeline );
   }

   return testResult;
}

static bool testCaseSocSinkBasicSeekZeroBased( EMCTX *emctx )
{
   bool testResult= false;
   EMSimpleVideoDecoder *videoDecoder= 0;

   videoDecoder= EMGetSimpleVideoDecoder( emctx, EM_TUNERID_MAIN );
   if ( !videoDecoder )
   {
      EMERROR("Failed to obtain test video decoder");
      goto exit;
   }

   EMSimpleVideoDecoderSetSegmentsStartAtZero( videoDecoder, true );

   testResult= testCaseSocSinkBasicSeek( emctx );

exit:

   return testResult;
}

static bool testCaseSocSinkServerPlaySpeedDecodeRate( EMCTX *emctx )
{
   bool testResult= false;
   int argc= 0;
   char **argv= 0;
   GstElement *pipeline= 0;
   GstElement *src= 0;
   GstElement *sink= 0;
   EMSimpleVideoDecoder *videoDecoder= 0;
   int stcChannelProxy;
   int videoPidChannelProxy;
   int decodeRate;

   videoDecoder= EMGetSimpleVideoDecoder( emctx, EM_TUNERID_MAIN );
   if ( !videoDecoder )
   {
      EMERROR("Failed to obtain test video decoder");
      goto exit;
   }

   EMSetStcChannel( emctx, (void*)&stcChannelProxy );
   EMSetVideoCodec( emctx, bvideo_codec_h264 );
   EMSetVideoPidChannel( emctx, (void*)&videoPidChannelProxy );
   EMSimpleVideoDecoderSetVideoSize( videoDecoder, 1920, 1080 );

   gst_init( &argc, &argv );

   pipeline= gst_pipeline_new("pipeline");
   if ( !pipeline )
   {
      EMERROR("Failed to create pipeline instance");
      goto exit;
   }

   src= createVideoSrc( emctx, videoDecoder );
   if ( !src )
   {
      EMERROR("Failed to create src instance");
      goto exit;
   }

   sink= gst_element_factory_make( "westerossink", "vsink" );
   if ( !sink )
   {
      EMERROR("Failed to create sink instance");
      goto exit;
   }

   gst_bin_add_many( GST_BIN(pipeline), src, sink, NULL );

   if ( gst_element_link( src, sink ) != TRUE )
   {
      EMERROR("Failed to link src and sink");
      goto exit;
   }

   gst_element_set_state( pipeline, GST_STATE_PLAYING );

   usleep( 200000 );

   EMSimpleVideoDecoderSetTrickStateRate( videoDecoder, 2*NEXUS_NORMAL_DECODE_RATE );

   g_object_set( G_OBJECT(sink), "server-play-speed", 2.0, NULL );

   usleep( 200000 );

   decodeRate= EMSimpleVideoDecoderGetTrickStateRate( videoDecoder );
   if ( decodeRate != NEXUS_NORMAL_DECODE_RATE )
   {
      EMERROR("Unexpected decode rate: expected(%d) actual(%d)", NEXUS_NORMAL_DECODE_RATE, decodeRate);
      goto exit;
   }

   gst_element_set_state( pipeline, GST_STATE_NULL );

   testResult= true;

exit:
   if ( pipeline )
   {
      gst_object_unref( pipeline );
   }

   return testResult;
}

static bool testCaseSocSinkInitWithCompositor( EMCTX *emctx )
{
   bool testResult= false;
   WstCompositor *wctx= 0;
   bool result;
   const char *value;

   wctx= WstCompositorCreate();
   if ( !wctx )
   {
      EMERROR( "WstCompositorCreate failed" );
      goto exit;
   }

   result= WstCompositorSetRendererModule( wctx, "libwesteros_render_gl.so.0.0.0" );
   if ( result == false )
   {
      EMERROR( "WstCompositorSetRenderedModule failed" );
      goto exit;
   }

   value= WstCompositorGetDisplayName( wctx );
   if ( value == 0 )
   {
      EMERROR( "WstCompositorGetDisplayName failed to return auto-generated name" );
      goto exit;
   }

   result= WstCompositorStart( wctx );
   if ( result == false )
   {
      EMERROR( "WstCompositorStart failed" );
      goto exit;
   }

   setenv( "WAYLAND_DISPLAY", value, true );

   testResult= testCaseSocSinkInit( emctx );

   WstCompositorDestroy( wctx );

exit:

   unsetenv( "WAYLAND_DISPLAY" );

   return testResult;
}

static bool testCaseSocSinkBasicPipelineWithCompositor( EMCTX *emctx )
{
   bool testResult= false;
   WstCompositor *wctx= 0;
   bool result;
   const char *value;

   wctx= WstCompositorCreate();
   if ( !wctx )
   {
      EMERROR( "WstCompositorCreate failed" );
      goto exit;
   }

   result= WstCompositorSetRendererModule( wctx, "libwesteros_render_gl.so.0.0.0" );
   if ( result == false )
   {
      EMERROR( "WstCompositorSetRenderedModule failed" );
      goto exit;
   }

   value= WstCompositorGetDisplayName( wctx );
   if ( value == 0 )
   {
      EMERROR( "WstCompositorGetDisplayName failed to return auto-generated name" );
      goto exit;
   }

   result= WstCompositorStart( wctx );
   if ( result == false )
   {
      EMERROR( "WstCompositorStart failed" );
      goto exit;
   }

   setenv( "WAYLAND_DISPLAY", value, true );

   testResult= testCaseSocSinkBasicPipeline( emctx );

   WstCompositorDestroy( wctx );

exit:

   unsetenv( "WAYLAND_DISPLAY" );

   return testResult;
}

static bool testCaseSocSinkFirstFrameSignalWithCompositor( EMCTX *emctx )
{
   bool testResult= false;
   WstCompositor *wctx= 0;
   bool result;
   const char *value;

   wctx= WstCompositorCreate();
   if ( !wctx )
   {
      EMERROR( "WstCompositorCreate failed" );
      goto exit;
   }

   result= WstCompositorSetRendererModule( wctx, "libwesteros_render_gl.so.0.0.0" );
   if ( result == false )
   {
      EMERROR( "WstCompositorSetRenderedModule failed" );
      goto exit;
   }

   value= WstCompositorGetDisplayName( wctx );
   if ( value == 0 )
   {
      EMERROR( "WstCompositorGetDisplayName failed to return auto-generated name" );
      goto exit;
   }

   result= WstCompositorStart( wctx );
   if ( result == false )
   {
      EMERROR( "WstCompositorStart failed" );
      goto exit;
   }

   setenv( "WAYLAND_DISPLAY", value, true );

   testResult= testCaseSocSinkFirstFrameSignal( emctx );

   WstCompositorDestroy( wctx );

exit:

   unsetenv( "WAYLAND_DISPLAY" );

   return testResult;
}

static bool testCaseSocSinkUnderflowSignalWithCompositor( EMCTX *emctx )
{
   bool testResult= false;
   WstCompositor *wctx= 0;
   bool result;
   const char *value;

   wctx= WstCompositorCreate();
   if ( !wctx )
   {
      EMERROR( "WstCompositorCreate failed" );
      goto exit;
   }

   result= WstCompositorSetRendererModule( wctx, "libwesteros_render_gl.so.0.0.0" );
   if ( result == false )
   {
      EMERROR( "WstCompositorSetRenderedModule failed" );
      goto exit;
   }

   value= WstCompositorGetDisplayName( wctx );
   if ( value == 0 )
   {
      EMERROR( "WstCompositorGetDisplayName failed to return auto-generated name" );
      goto exit;
   }

   result= WstCompositorStart( wctx );
   if ( result == false )
   {
      EMERROR( "WstCompositorStart failed" );
      goto exit;
   }

   setenv( "WAYLAND_DISPLAY", value, true );

   testResult= testCaseSocSinkUnderflowSignal( emctx );

   WstCompositorDestroy( wctx );

exit:

   unsetenv( "WAYLAND_DISPLAY" );

   return testResult;
}

static bool testCaseSocSinkPtsErrorSignalWithCompositor( EMCTX *emctx )
{
   bool testResult= false;
   WstCompositor *wctx= 0;
   bool result;
   const char *value;

   wctx= WstCompositorCreate();
   if ( !wctx )
   {
      EMERROR( "WstCompositorCreate failed" );
      goto exit;
   }

   result= WstCompositorSetRendererModule( wctx, "libwesteros_render_gl.so.0.0.0" );
   if ( result == false )
   {
      EMERROR( "WstCompositorSetRenderedModule failed" );
      goto exit;
   }

   value= WstCompositorGetDisplayName( wctx );
   if ( value == 0 )
   {
      EMERROR( "WstCompositorGetDisplayName failed to return auto-generated name" );
      goto exit;
   }

   result= WstCompositorStart( wctx );
   if ( result == false )
   {
      EMERROR( "WstCompositorStart failed" );
      goto exit;
   }

   setenv( "WAYLAND_DISPLAY", value, true );

   testResult= testCaseSocSinkPtsErrorSignal( emctx );

   WstCompositorDestroy( wctx );

exit:

   unsetenv( "WAYLAND_DISPLAY" );

   return testResult;
}

static bool testCaseSocSinkBasicPositionReportingWithCompositor( EMCTX *emctx )
{
   bool testResult= false;
   WstCompositor *wctx= 0;
   bool result;
   const char *value;

   wctx= WstCompositorCreate();
   if ( !wctx )
   {
      EMERROR( "WstCompositorCreate failed" );
      goto exit;
   }

   result= WstCompositorSetRendererModule( wctx, "libwesteros_render_gl.so.0.0.0" );
   if ( result == false )
   {
      EMERROR( "WstCompositorSetRenderedModule failed" );
      goto exit;
   }

   value= WstCompositorGetDisplayName( wctx );
   if ( value == 0 )
   {
      EMERROR( "WstCompositorGetDisplayName failed to return auto-generated name" );
      goto exit;
   }

   result= WstCompositorStart( wctx );
   if ( result == false )
   {
      EMERROR( "WstCompositorStart failed" );
      goto exit;
   }

   setenv( "WAYLAND_DISPLAY", value, true );

   testResult= testCaseSocSinkBasicPositionReporting( emctx );

   WstCompositorDestroy( wctx );

exit:

   unsetenv( "WAYLAND_DISPLAY" );

   return testResult;
}

static bool testCaseSocSinkBasicPauseResumeWithCompositor( EMCTX *emctx )
{
   bool testResult= false;
   WstCompositor *wctx= 0;
   bool result;
   const char *value;

   wctx= WstCompositorCreate();
   if ( !wctx )
   {
      EMERROR( "WstCompositorCreate failed" );
      goto exit;
   }

   result= WstCompositorSetRendererModule( wctx, "libwesteros_render_gl.so.0.0.0" );
   if ( result == false )
   {
      EMERROR( "WstCompositorSetRenderedModule failed" );
      goto exit;
   }

   value= WstCompositorGetDisplayName( wctx );
   if ( value == 0 )
   {
      EMERROR( "WstCompositorGetDisplayName failed to return auto-generated name" );
      goto exit;
   }

   result= WstCompositorStart( wctx );
   if ( result == false )
   {
      EMERROR( "WstCompositorStart failed" );
      goto exit;
   }

   setenv( "WAYLAND_DISPLAY", value, true );

   testResult= testCaseSocSinkBasicPauseResume( emctx );

   WstCompositorDestroy( wctx );

exit:

   unsetenv( "WAYLAND_DISPLAY" );

   return testResult;
}

static bool testCaseSocSinkBasicSeekWithCompositor( EMCTX *emctx )
{
   bool testResult= false;
   WstCompositor *wctx= 0;
   bool result;
   const char *value;

   wctx= WstCompositorCreate();
   if ( !wctx )
   {
      EMERROR( "WstCompositorCreate failed" );
      goto exit;
   }

   result= WstCompositorSetRendererModule( wctx, "libwesteros_render_gl.so.0.0.0" );
   if ( result == false )
   {
      EMERROR( "WstCompositorSetRenderedModule failed" );
      goto exit;
   }

   value= WstCompositorGetDisplayName( wctx );
   if ( value == 0 )
   {
      EMERROR( "WstCompositorGetDisplayName failed to return auto-generated name" );
      goto exit;
   }

   result= WstCompositorStart( wctx );
   if ( result == false )
   {
      EMERROR( "WstCompositorStart failed" );
      goto exit;
   }

   setenv( "WAYLAND_DISPLAY", value, true );

   testResult= testCaseSocSinkBasicSeek( emctx );

   WstCompositorDestroy( wctx );

exit:

   unsetenv( "WAYLAND_DISPLAY" );

   return testResult;
}

static bool testCaseSocSinkBasicSeekZeroBasedWithCompositor( EMCTX *emctx )
{
   bool testResult= false;
   WstCompositor *wctx= 0;
   bool result;
   const char *value;

   wctx= WstCompositorCreate();
   if ( !wctx )
   {
      EMERROR( "WstCompositorCreate failed" );
      goto exit;
   }

   result= WstCompositorSetRendererModule( wctx, "libwesteros_render_gl.so.0.0.0" );
   if ( result == false )
   {
      EMERROR( "WstCompositorSetRenderedModule failed" );
      goto exit;
   }

   value= WstCompositorGetDisplayName( wctx );
   if ( value == 0 )
   {
      EMERROR( "WstCompositorGetDisplayName failed to return auto-generated name" );
      goto exit;
   }

   result= WstCompositorStart( wctx );
   if ( result == false )
   {
      EMERROR( "WstCompositorStart failed" );
      goto exit;
   }

   setenv( "WAYLAND_DISPLAY", value, true );

   testResult= testCaseSocSinkBasicSeekZeroBased( emctx );

   WstCompositorDestroy( wctx );

exit:

   unsetenv( "WAYLAND_DISPLAY" );

   return testResult;
}

namespace SocSimpleShell
{

typedef struct _TestCtx
{
   TestEGLCtx eglCtx;
   struct wl_display *display;
   struct wl_compositor *compositor;
   struct wl_seat *seat;
   struct wl_pointer *pointer;
   struct wl_keyboard *keyboard;
   struct wl_simple_shell *shell;
   int windowWidth;
   int windowHeight;
   struct xkb_context *xkbCtx;
   struct xkb_keymap *xkbKeymap;
   struct xkb_state *xkbState;
   xkb_mod_index_t modAlt;
   xkb_mod_index_t modCtrl;
   xkb_mod_index_t modShift;
   xkb_mod_index_t modCaps;
   unsigned int modMask;

   struct wl_surface *surface1;
   uint32_t surfaceId1;
   char surfaceName1[33];
   struct wl_egl_window *wlEglWindow1;
   EGLSurface eglSurfaceWindow1;
   uint32_t surfaceVisible1;
   int32_t surfaceX1;
   int32_t surfaceY1;
   int32_t surfaceW1;
   int32_t surfaceH1;
   float surfaceOpacity1;
   float surfaceZOrder1;

   struct wl_surface *surface2;
   uint32_t surfaceId2;
   char surfaceName2[33];
   struct wl_egl_window *wlEglWindow2;
   EGLSurface eglSurfaceWindow2;
   uint32_t surfaceVisible2;
   int32_t surfaceX2;
   int32_t surfaceY2;
   int32_t surfaceW2;
   int32_t surfaceH2;
   float surfaceOpacity2;
   float surfaceZOrder2;

   bool keyboardMap;
   bool keyboardEnter;
   bool keyboardLeave;
   bool keyboardRepeatInfo;
   bool keyAlt;
   bool keyCtrl;
   bool keyShift;
   bool keyCaps;
   int keyPressed;
   struct wl_surface *surfaceWithKeyInput;

   bool unexpectedSurfaceId;
   uint32_t surfaceCreatedId;
   uint32_t surfaceDestroyedId;
   int surfaceListCount;
   bool surfaceListDone;
} TestCtx;

static void pointerEnter( void* data, struct wl_pointer *pointer, uint32_t serial,
                          struct wl_surface *surface, wl_fixed_t sx, wl_fixed_t sy )
{
   TestCtx *ctx= (TestCtx*)data;
}

static void pointerLeave( void* data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface )
{
   TestCtx *ctx= (TestCtx*)data;
}

static void pointerMotion( void *data, struct wl_pointer *pointer, uint32_t time, wl_fixed_t sx, wl_fixed_t sy )
{
}

static void pointerButton( void *data, struct wl_pointer *pointer, uint32_t serial,
                           uint32_t time, uint32_t button, uint32_t state )
{
}

static void pointerAxis( void *data, struct wl_pointer *pointer, uint32_t time,
                         uint32_t axis, wl_fixed_t value )
{
}

static const struct wl_pointer_listener pointerListener = {
   pointerEnter,
   pointerLeave,
   pointerMotion,
   pointerButton,
   pointerAxis
};

static void keyboardKeymap( void *data, struct wl_keyboard *keyboard, uint32_t format, int32_t fd, uint32_t size )
{
   TestCtx *ctx= (TestCtx*)data;

   if ( format == WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1 )
   {
      void *map= mmap( 0, size, PROT_READ, MAP_SHARED, fd, 0 );
      if ( map != MAP_FAILED )
      {
         if ( !ctx->xkbCtx )
         {
            ctx->xkbCtx= xkb_context_new( XKB_CONTEXT_NO_FLAGS );
         }
         else
         {
            printf("error: xkb_context_new failed\n");
         }
         if ( ctx->xkbCtx )
         {
            if ( ctx->xkbKeymap )
            {
               xkb_keymap_unref( ctx->xkbKeymap );
               ctx->xkbKeymap= 0;
            }
            ctx->xkbKeymap= xkb_keymap_new_from_string( ctx->xkbCtx, (char*)map, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
            if ( !ctx->xkbKeymap )
            {
               printf("error: xkb_keymap_new_from_string failed\n");
            }
            if ( ctx->xkbState )
            {
               xkb_state_unref( ctx->xkbState );
               ctx->xkbState= 0;
            }
            ctx->xkbState= xkb_state_new( ctx->xkbKeymap );
            if ( !ctx->xkbState )
            {
               printf("error: xkb_state_new failed\n");
            }
            if ( ctx->xkbKeymap )
            {
               ctx->modAlt= xkb_keymap_mod_get_index( ctx->xkbKeymap, XKB_MOD_NAME_ALT );
               ctx->modCtrl= xkb_keymap_mod_get_index( ctx->xkbKeymap, XKB_MOD_NAME_CTRL );
               ctx->modShift= xkb_keymap_mod_get_index( ctx->xkbKeymap, XKB_MOD_NAME_SHIFT );
               ctx->modCaps= xkb_keymap_mod_get_index( ctx->xkbKeymap, XKB_MOD_NAME_CAPS );
            }
            munmap( map, size );

            ctx->keyboardMap= true;
         }
      }
   }

   close( fd );
}
static void keyboardEnter( void *data, struct wl_keyboard *keyboard, uint32_t serial,
                           struct wl_surface *surface, struct wl_array *keys )
{
   TestCtx *ctx= (TestCtx*)data;
   printf("keyboard enter\n");
   ctx->keyboardEnter= true;
   ctx->surfaceWithKeyInput= surface;
}

static void keyboardLeave( void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface )
{
   TestCtx *ctx= (TestCtx*)data;
   printf("keyboard leave\n");
   ctx->keyboardLeave= true;
   ctx->surfaceWithKeyInput= 0;
}

static void keyboardKey( void *data, struct wl_keyboard *keyboard, uint32_t serial,
                         uint32_t time, uint32_t key, uint32_t state )
{
   TestCtx *ctx= (TestCtx*)data;
   if ( state == WL_KEYBOARD_KEY_STATE_PRESSED )
   {
      ctx->keyPressed= key;
   }
   else if ( state == WL_KEYBOARD_KEY_STATE_RELEASED )
   {
      ctx->keyPressed= 0;
   }
}

static void keyboardModifiers( void *data, struct wl_keyboard *keyboard, uint32_t serial,
                               uint32_t mods_depressed, uint32_t mods_latched,
                               uint32_t mods_locked, uint32_t group )
{
   TestCtx *ctx= (TestCtx*)data;
   int wasActive, nowActive;
   if ( ctx->xkbState )
   {
      xkb_state_update_mask( ctx->xkbState, mods_depressed, mods_latched, mods_locked, 0, 0, group );

      wasActive= (ctx->modMask & (1<<ctx->modAlt));
      nowActive= (mods_depressed & (1<<ctx->modAlt));
      if ( nowActive != wasActive )
      {
         ctx->modMask ^= (1<<ctx->modAlt);
         ctx->keyAlt= nowActive;
      }

      wasActive= (ctx->modMask & (1<<ctx->modCtrl));
      nowActive= (mods_depressed & (1<<ctx->modCtrl));
      if ( nowActive != wasActive )
      {
         ctx->modMask ^= (1<<ctx->modCtrl);
         ctx->keyCtrl= nowActive;
      }

      wasActive= (ctx->modMask & (1<<ctx->modShift));
      nowActive= (mods_depressed & (1<<ctx->modShift));
      if ( nowActive != wasActive )
      {
         ctx->modMask ^= (1<<ctx->modShift);
         ctx->keyShift= nowActive;
      }

      wasActive= (ctx->modMask & (1<<ctx->modCaps));
      nowActive= (mods_locked & (1<<ctx->modCaps));
      if ( nowActive != wasActive )
      {
         ctx->modMask ^= (1<<ctx->modCaps);
         ctx->keyCaps= nowActive;
      }
   }
}

static void keyboardRepeatInfo( void *data, struct wl_keyboard *keyboard, int32_t rate, int32_t delay )
{
   TestCtx *ctx= (TestCtx*)data;
   ctx->keyboardRepeatInfo= true;
}

static const struct wl_keyboard_listener keyboardListener= {
   keyboardKeymap,
   keyboardEnter,
   keyboardLeave,
   keyboardKey,
   keyboardModifiers,
   keyboardRepeatInfo
};

static void seatCapabilities( void *data, struct wl_seat *seat, uint32_t capabilities )
{
   TestCtx *ctx= (TestCtx*)data;
   if ( capabilities & WL_SEAT_CAPABILITY_KEYBOARD )
   {
      ctx->keyboard= wl_seat_get_keyboard( ctx->seat );
      printf("keyboard %p\n", ctx->keyboard );
      wl_keyboard_add_listener( ctx->keyboard, &keyboardListener, ctx );
      wl_display_roundtrip(ctx->display);
   }
   if ( capabilities & WL_SEAT_CAPABILITY_POINTER )
   {
      ctx->pointer= wl_seat_get_pointer( ctx->seat );
      printf("pointer %p\n", ctx->pointer );
      wl_pointer_add_listener( ctx->pointer, &pointerListener, ctx );
      wl_display_roundtrip(ctx->display);
   }
}

static void seatName( void *data, struct wl_seat *seat, const char *name )
{
}

static const struct wl_seat_listener seatListener = {
   seatCapabilities,
   seatName
};

static void shellSurfaceId(void *data,
                           struct wl_simple_shell *wl_simple_shell,
                           struct wl_surface *surface,
                           uint32_t surfaceId)
{
   TestCtx *ctx= (TestCtx*)data;
   if ( surface == ctx->surface1 )
   {
      ctx->surfaceId1= surfaceId;
      sprintf( ctx->surfaceName1, "test-surface-%x", surfaceId );
      wl_simple_shell_set_name( ctx->shell, surfaceId, ctx->surfaceName1 );
   }
   else if ( surface == ctx->surface2 )
   {
      ctx->surfaceId2= surfaceId;
      sprintf( ctx->surfaceName2, "test-surface-%x", surfaceId );
      wl_simple_shell_set_name( ctx->shell, surfaceId, ctx->surfaceName2 );
   }
   else
   {
      ctx->unexpectedSurfaceId= true;
   }
}

static void shellSurfaceCreated(void *data,
                                struct wl_simple_shell *wl_simple_shell,
                                uint32_t surfaceId,
                                const char *name)
{
   TestCtx *ctx= (TestCtx*)data;
   printf("shellSurfaceCreated: id %d name (%s)\n", surfaceId, name );
   ctx->surfaceCreatedId= surfaceId;
}

static void shellSurfaceDestroyed(void *data,
                                  struct wl_simple_shell *wl_simple_shell,
                                  uint32_t surfaceId,
                                  const char *name)
{
   TestCtx *ctx= (TestCtx*)data;
   ctx->surfaceDestroyedId= surfaceId;
}

static void shellSurfaceStatus(void *data,
                               struct wl_simple_shell *wl_simple_shell,
                               uint32_t surfaceId,
                               const char *name,
                               uint32_t visible,
                               int32_t x,
                               int32_t y,
                               int32_t width,
                               int32_t height,
                               wl_fixed_t opacity,
                               wl_fixed_t zorder)
{
   TestCtx *ctx= (TestCtx*)data;
   if ( surfaceId == ctx->surfaceId1 )
   {
      ctx->surfaceVisible1= visible;
      ctx->surfaceX1= x;
      ctx->surfaceY1= y;
      ctx->surfaceW1= width;
      ctx->surfaceH1= height;
      ctx->surfaceOpacity1= wl_fixed_to_double( opacity );
      ctx->surfaceZOrder1= wl_fixed_to_double( zorder );
   }
   else if ( surfaceId == ctx->surfaceId2 )
   {
      ctx->surfaceVisible2= visible;
      ctx->surfaceX2= x;
      ctx->surfaceY2= y;
      ctx->surfaceW2= width;
      ctx->surfaceH2= height;
      ctx->surfaceOpacity2= wl_fixed_to_double( opacity );
      ctx->surfaceZOrder2= wl_fixed_to_double( zorder );
   }
   else
   {
      ctx->unexpectedSurfaceId= true;
   }
   ++ctx->surfaceListCount;
}

static void shellGetSurfacesDone(void *data,
                                 struct wl_simple_shell *wl_simple_shell)
{
   TestCtx *ctx= (TestCtx*)data;
   ctx->surfaceListDone= true;
}

static const struct wl_simple_shell_listener shellListener = 
{
   shellSurfaceId,
   shellSurfaceCreated,
   shellSurfaceDestroyed,
   shellSurfaceStatus,
   shellGetSurfacesDone
};

static void registryHandleGlobal(void *data, 
                                 struct wl_registry *registry, uint32_t id,
                                 const char *interface, uint32_t version)
{
   TestCtx *ctx= (TestCtx*)data;
   int len;

   len= strlen(interface);

   if ( (len==13) && !strncmp(interface, "wl_compositor", len) ) {
      ctx->compositor= (struct wl_compositor*)wl_registry_bind(registry, id, &wl_compositor_interface, 1);
      printf("compositor %p\n", ctx->compositor);
   }
   else if ( (len==7) && !strncmp(interface, "wl_seat", len) ) {
      ctx->seat= (struct wl_seat*)wl_registry_bind(registry, id, &wl_seat_interface, 4);
      printf("seat %p\n", ctx->seat);
      wl_seat_add_listener(ctx->seat, &seatListener, ctx);
      wl_display_roundtrip(ctx->display);
   }
   else if ( (len==15) && !strncmp(interface, "wl_simple_shell", len) ) {
      ctx->shell= (struct wl_simple_shell*)wl_registry_bind(registry, id, &wl_simple_shell_interface, 1);      
      printf("shell %p\n", ctx->shell );
      wl_simple_shell_add_listener(ctx->shell, &shellListener, ctx);
   }
}

static void registryHandleGlobalRemove(void *data, 
                                      struct wl_registry *registry,
                                      uint32_t name)
{
}

static const struct wl_registry_listener registryListener = 
{
   registryHandleGlobal,
   registryHandleGlobalRemove
};

} // namespace SocSimpleShell

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

bool testCaseSocSimpleShellBasic( EMCTX *emctx )
{
   using namespace SocSimpleShell;

   bool testResult= false;
   bool result;
   WstCompositor *wctx= 0;
   const char *displayName= "test0";
   struct wl_display *display= 0;
   struct wl_registry *registry= 0;
   TestCtx testCtx;
   TestCtx *ctx= &testCtx;
   EGLBoolean b;
   float threshold= 0.001;
   int gx, gy, gw, gh;
   float opacity;
   float zorder;

   wctx= WstCompositorCreate();
   if ( !wctx )
   {
      EMERROR( "WstCompositorCreate failed" );
      goto exit;
   }

   result= WstCompositorSetDisplayName( wctx, displayName );
   if ( result == false )
   {
      EMERROR( "WstCompositorSetDisplayName failed" );
      goto exit;
   }

   result= WstCompositorSetRendererModule( wctx, "libwesteros_render_nexus.so.0.0.0" );
   if ( result == false )
   {
      EMERROR( "WstCompositorSetRendererModule failed" );
      goto exit;
   }

   result= WstCompositorStart( wctx );
   if ( result == false )
   {
      EMERROR( "WstCompositorStart failed" );
      goto exit;
   }

   memset( &testCtx, 0, sizeof(TestCtx) );

   display= wl_display_connect(displayName);
   if ( !display )
   {
      EMERROR( "wl_display_connect failed" );
      goto exit;
   }
   ctx->display= display;

   registry= wl_display_get_registry(display);
   if ( !registry )
   {
      EMERROR( "wl_display_get_registrty failed" );
      goto exit;
   }

   wl_registry_add_listener(registry, &registryListener, ctx);

   wl_display_roundtrip(display);

   if ( !ctx->compositor || !ctx->seat || !ctx->keyboard || !ctx->pointer || !ctx->shell )
   {
      EMERROR("Failed to acquire needed compositor items");
      goto exit;
   }

   result= testSetupEGL( &ctx->eglCtx, display );
   if ( !result )
   {
      EMERROR("testSetupEGL failed");
      goto exit;
   }

   ctx->windowWidth= WINDOW_WIDTH;
   ctx->windowHeight= WINDOW_HEIGHT;

   eglSwapInterval( ctx->eglCtx.eglDisplay, 1 );




   ctx->surface1= wl_compositor_create_surface(ctx->compositor);
   printf("surface1=%p\n", ctx->surface1);   
   if ( !ctx->surface1 )
   {
      EMERROR("error: unable to create wayland surface");
      goto exit;
   }

   ctx->wlEglWindow1= wl_egl_window_create(ctx->surface1, ctx->windowWidth, ctx->windowHeight);
   if ( !ctx->wlEglWindow1 )
   {
      EMERROR("error: unable to create wl_egl_window");
      goto exit;
   }
   printf("wl_egl_window1 %p\n", ctx->wlEglWindow1);

   ctx->eglSurfaceWindow1= eglCreateWindowSurface( ctx->eglCtx.eglDisplay,
                                                   ctx->eglCtx.eglConfig,
                                                   (EGLNativeWindowType)ctx->wlEglWindow1,
                                                   NULL );
   printf("eglCreateWindowSurface: eglSurfaceWindow1 %p\n", ctx->eglSurfaceWindow1 );

   b= eglMakeCurrent( ctx->eglCtx.eglDisplay, ctx->eglSurfaceWindow1, ctx->eglSurfaceWindow1, ctx->eglCtx.eglContext );
   if ( !b )
   {
      EMERROR("error: eglMakeCurrent failed: %X", eglGetError() );
      goto exit;
   }

   eglSwapBuffers(ctx->eglCtx.eglDisplay, ctx->eglSurfaceWindow1);

   wl_display_roundtrip(display);

   usleep( 33000 );

   if ( ctx->unexpectedSurfaceId )
   {
      EMERROR("Received unexpected surface id");
      goto exit;
   }

   if ( ctx->surfaceId1 == 0 )
   {
      EMERROR("Did not get surface id for surface 1");
      goto exit;
   }




   ctx->surfaceCreatedId= 0;

   ctx->surface2= wl_compositor_create_surface(ctx->compositor);
   printf("surface2=%p\n", ctx->surface2);   
   if ( !ctx->surface2 )
   {
      EMERROR("error: unable to create wayland surface");
      goto exit;
   }

   ctx->wlEglWindow2= wl_egl_window_create(ctx->surface2, ctx->windowWidth, ctx->windowHeight);
   if ( !ctx->wlEglWindow2 )
   {
      EMERROR("error: unable to create wl_egl_window");
      goto exit;
   }
   printf("wl_egl_window2 %p\n", ctx->wlEglWindow2);

   ctx->eglSurfaceWindow2= eglCreateWindowSurface( ctx->eglCtx.eglDisplay,
                                                   ctx->eglCtx.eglConfig,
                                                   (EGLNativeWindowType)ctx->wlEglWindow2,
                                                   NULL );
   printf("eglCreateWindowSurface: eglSurfaceWindow2 %p\n", ctx->eglSurfaceWindow2 );

   b= eglMakeCurrent( ctx->eglCtx.eglDisplay, ctx->eglSurfaceWindow2, ctx->eglSurfaceWindow2, ctx->eglCtx.eglContext );
   if ( !b )
   {
      EMERROR("error: eglMakeCurrent failed: %X", eglGetError() );
      goto exit;
   }

   eglSwapBuffers(ctx->eglCtx.eglDisplay, ctx->eglSurfaceWindow2);

   wl_display_roundtrip(display);

   usleep( 33000 );

   if ( ctx->unexpectedSurfaceId )
   {
      EMERROR("Received unexpected surface id");
      goto exit;
   }

   if ( ctx->surfaceId2 == 0 )
   {
      EMERROR("Did not get surface id for surface 2");
      goto exit;
   }

   if ( ctx->surfaceCreatedId != ctx->surfaceId2 )
   {
      EMERROR("Did not get surface created event for surface 2");
      goto exit;
   }


   // Visibility
   wl_simple_shell_set_visible( ctx->shell, ctx->surfaceId1, 0);

   wl_display_roundtrip(display);

   wl_simple_shell_get_status( ctx->shell, ctx->surfaceId1 );

   wl_display_roundtrip(display);

   if ( ctx->surfaceVisible1 != 0 )
   {
      EMERROR("Failed to make surface1 non-visible");
      goto exit;
   }

   wl_simple_shell_set_visible( ctx->shell, ctx->surfaceId1, 1);

   wl_display_roundtrip(display);

   wl_simple_shell_get_status( ctx->shell, ctx->surfaceId1 );

   wl_display_roundtrip(display);

   if ( ctx->surfaceVisible1 != 1 )
   {
      EMERROR("Failed to make surface1 visible");
      goto exit;
   }

   if ( ctx->unexpectedSurfaceId )
   {
      EMERROR("Received unexpected surface id");
      goto exit;
   }

   wl_simple_shell_set_visible( ctx->shell, ctx->surfaceId2, 0);

   wl_display_roundtrip(display);

   wl_simple_shell_get_status( ctx->shell, ctx->surfaceId2 );

   wl_display_roundtrip(display);

   if ( ctx->surfaceVisible2 != 0 )
   {
      EMERROR("Failed to make surface2 non-visible");
      goto exit;
   }

   wl_simple_shell_set_visible( ctx->shell, ctx->surfaceId2, 1);

   wl_display_roundtrip(display);

   wl_simple_shell_get_status( ctx->shell, ctx->surfaceId2 );

   wl_display_roundtrip(display);

   if ( ctx->surfaceVisible2 != 1 )
   {
      EMERROR("Failed to make surface2 visible");
      goto exit;
   }

   if ( ctx->unexpectedSurfaceId )
   {
      EMERROR("Received unexpected surface id");
      goto exit;
   }


   // Geometry
   gx= 10;
   gy= 40;
   gw= 200;
   gh= 300;
   wl_simple_shell_set_geometry( ctx->shell, ctx->surfaceId1, gx, gy, gw, gh);

   wl_display_roundtrip(display);

   wl_simple_shell_get_status( ctx->shell, ctx->surfaceId1 );

   wl_display_roundtrip(display);

   if ( (ctx->surfaceX1 != gx) || (ctx->surfaceY1 != gy) || (ctx->surfaceW1 != gw) || (ctx->surfaceH1 != gh) )
   {
      EMERROR("Failed to set surface1 geometry: expected (%d,%d,%d,%d) actual (%d,%d,%d,%d)",
               gx, gy, gw, gh, ctx->surfaceX1, ctx->surfaceY1, ctx->surfaceW1, ctx->surfaceH1 );
      goto exit;
   }

   gx= 20;
   gy= 50;
   gw= 300;
   gh= 400;
   wl_simple_shell_set_geometry( ctx->shell, ctx->surfaceId2, gx, gy, gw, gh);

   wl_display_roundtrip(display);

   wl_simple_shell_get_status( ctx->shell, ctx->surfaceId2 );

   wl_display_roundtrip(display);

   if ( (ctx->surfaceX2 != gx) || (ctx->surfaceY2 != gy) || (ctx->surfaceW2 != gw) || (ctx->surfaceH2 != gh) )
   {
      EMERROR("Failed to set surface2 geometry: expected (%d,%d,%d,%d) actual (%d,%d,%d,%d)",
               gx, gy, gw, gh, ctx->surfaceX2, ctx->surfaceY2, ctx->surfaceW2, ctx->surfaceH2 );
      goto exit;
   }

   if ( ctx->unexpectedSurfaceId )
   {
      EMERROR("Received unexpected surface id");
      goto exit;
   }

   // Opacity
   opacity= 0.5;
   wl_simple_shell_set_opacity( ctx->shell, ctx->surfaceId1, wl_fixed_from_double(opacity) );

   wl_display_roundtrip(display);

   wl_simple_shell_get_status( ctx->shell, ctx->surfaceId1 );

   wl_display_roundtrip(display);

   if ( (ctx->surfaceOpacity1 - opacity) > threshold )
   {
      EMERROR("Failed to set surface1 opacity: expected (%f) actual (%f)", opacity, ctx->surfaceOpacity1 );
      goto exit;
   }

   opacity= 0.25;
   wl_simple_shell_set_opacity( ctx->shell, ctx->surfaceId2, wl_fixed_from_double(opacity) );

   wl_display_roundtrip(display);

   wl_simple_shell_get_status( ctx->shell, ctx->surfaceId2 );

   wl_display_roundtrip(display);

   if ( (ctx->surfaceOpacity2 - opacity) > threshold )
   {
      EMERROR("Failed to set surface2 opacity: expected (%f) actual (%f)", opacity, ctx->surfaceOpacity2 );
      goto exit;
   }

   if ( ctx->unexpectedSurfaceId )
   {
      EMERROR("Received unexpected surface id");
      goto exit;
   }

   // ZOrder
   zorder= 0.5;
   wl_simple_shell_set_zorder( ctx->shell, ctx->surfaceId1, wl_fixed_from_double(zorder) );

   wl_display_roundtrip(display);

   wl_simple_shell_get_status( ctx->shell, ctx->surfaceId1 );

   wl_display_roundtrip(display);

   if ( (ctx->surfaceZOrder1 - zorder) > threshold )
   {
      EMERROR("Failed to set surface1 zorder: expected (%f) actual (%f)", opacity, ctx->surfaceZOrder1 );
      goto exit;
   }

   zorder= 0.25;
   wl_simple_shell_set_zorder( ctx->shell, ctx->surfaceId2, wl_fixed_from_double(zorder) );

   wl_display_roundtrip(display);

   wl_simple_shell_get_status( ctx->shell, ctx->surfaceId2 );

   wl_display_roundtrip(display);

   if ( (ctx->surfaceZOrder2 - zorder) > threshold )
   {
      EMERROR("Failed to set surface2 zorder: expected (%f) actual (%f)", opacity, ctx->surfaceZOrder2 );
      goto exit;
   }

   // List surfaces
   ctx->surfaceListCount= 0;
   ctx->surfaceListDone= false;
   wl_simple_shell_get_surfaces( ctx->shell );
   wl_display_roundtrip(display);

   usleep( 35000 );

   wl_display_roundtrip(display);

   if ( !ctx->surfaceListDone || (ctx->surfaceListCount != 2) )
   {
      EMERROR( "Error listing surfaces: list done: expected (1) actual (%d), surface count: expected (2), actual (%d)",
                ctx->surfaceListDone, ctx->surfaceListCount );
      goto exit;
   }


   // Key input focus
   wl_simple_shell_set_focus( ctx->shell, ctx->surfaceId2 );

   wl_display_roundtrip(display);

   WstCompositorKeyEvent( wctx,  KEY_D, WstKeyboard_keyState_depressed, 0 );

   wl_display_roundtrip(display);

   usleep( 35000 );

   wl_display_roundtrip(display);

   if ( (ctx->surfaceWithKeyInput != ctx->surface2) || (ctx->keyPressed != KEY_D) )
   {
      EMERROR("Failed to get key input with expected surface");
      goto exit;
   }

   WstCompositorKeyEvent( wctx,  KEY_D, WstKeyboard_keyState_released, 0 );

   wl_display_roundtrip(display);

   usleep( 35000 );

   wl_display_roundtrip(display);


   wl_simple_shell_set_focus( ctx->shell, ctx->surfaceId1 );

   wl_display_roundtrip(display);

   WstCompositorKeyEvent( wctx,  KEY_J, WstKeyboard_keyState_depressed, 0 );

   wl_display_roundtrip(display);

   usleep( 35000 );

   wl_display_roundtrip(display);

   if ( (ctx->surfaceWithKeyInput != ctx->surface1) || (ctx->keyPressed != KEY_J) )
   {
      EMERROR("Failed to get key input with expected surface");
      goto exit;
   }

   WstCompositorKeyEvent( wctx,  KEY_J, WstKeyboard_keyState_released, 0 );

   wl_display_roundtrip(display);

   usleep( 35000 );

   wl_display_roundtrip(display);



   // Surface destroyed
   ctx->surfaceDestroyedId= 0;
   eglDestroySurface( ctx->eglCtx.eglDisplay, ctx->eglSurfaceWindow2 );
   ctx->eglSurfaceWindow2= EGL_NO_SURFACE;
   wl_egl_window_destroy( ctx->wlEglWindow2 );
   ctx->wlEglWindow2= 0;
   wl_surface_destroy( ctx->surface2 );
   ctx->surface2= 0;

   wl_display_roundtrip(display);

   if ( ctx->surfaceDestroyedId != ctx->surfaceId2 )
   {
      EMERROR("Did not get surface created event for surface 2");
      goto exit;
   }

   if ( EMGetWaylandThreadingIssue( emctx ) )
   {
      EMERROR( "Wayland threading issue: compositor calling wl_resource_post_event_array from multiple threads") ;
      goto exit;
   }


   testResult= true;

exit:

   if ( ctx->eglSurfaceWindow2 )
   {
      eglDestroySurface( ctx->eglCtx.eglDisplay, ctx->eglSurfaceWindow2 );
      ctx->eglSurfaceWindow2= EGL_NO_SURFACE;
   }

   if ( ctx->wlEglWindow2 )
   {
      wl_egl_window_destroy( ctx->wlEglWindow2 );
      ctx->wlEglWindow2= 0;
   }

   if ( ctx->surface2 )
   {
      wl_surface_destroy( ctx->surface2 );
      ctx->surface2= 0;
   }

   if ( ctx->eglSurfaceWindow1 )
   {
      eglDestroySurface( ctx->eglCtx.eglDisplay, ctx->eglSurfaceWindow1 );
      ctx->eglSurfaceWindow1= EGL_NO_SURFACE;
   }

   if ( ctx->wlEglWindow1 )
   {
      wl_egl_window_destroy( ctx->wlEglWindow1 );
      ctx->wlEglWindow1= 0;
   }

   if ( ctx->surface1 )
   {
      wl_surface_destroy( ctx->surface1 );
      ctx->surface1= 0;
   }

   testTermEGL( &ctx->eglCtx );

   if ( ctx->keyboard )
   {
      wl_keyboard_destroy(ctx->keyboard);
      ctx->keyboard= 0;
   }

   if ( ctx->pointer )
   {
      wl_pointer_destroy(ctx->pointer);
      ctx->pointer= 0;
   }

   if ( ctx->seat )
   {
      wl_seat_destroy(ctx->seat);
      ctx->seat= 0;
   }

   if ( ctx->shell )
   {
      wl_simple_shell_destroy(ctx->shell);
      ctx->shell= 0;
   }

   if ( ctx->compositor )
   {
      wl_compositor_destroy( ctx->compositor );
      ctx->compositor= 0;
   }

   if ( registry )
   {
      wl_registry_destroy(registry);
      registry= 0;
   }

   if ( display )
   {
      wl_display_disconnect(display);
      display= 0;
   }

   WstCompositorDestroy( wctx );

   return testResult;
}

static bool testCaseSocAPISetDefaultCursor( EMCTX *emctx )
{
   bool testResult= false;
   bool result;
   unsigned char *imgData= 0;
   int cursorWidth, cursorHeight;
   int hotSpotX, hotSpotY;
   int imgDataSize;
   const char *displayName= "display0";
   const char *displayName2= "display1";
   WstCompositor *wctx= 0;
   WstCompositor *wctx2= 0;

   hotSpotX= 0;
   hotSpotY= 0;
   cursorWidth= 32;
   cursorHeight= 32;
   imgDataSize= cursorWidth*cursorHeight*4;

   imgData= (unsigned char*)malloc( imgDataSize );
   if ( !imgData )
   {
      EMERROR("No memory for image data");
      goto exit;
   }

   result= WstCompositorSetDefaultCursor( (WstCompositor*)0, imgData, cursorWidth, cursorHeight, hotSpotX, hotSpotY );
   if ( result )
   {
      EMERROR( "WstCompositorSetDefaultCursor did not fail with null handle" );
      goto exit;
   }

   wctx= WstCompositorCreate();
   if ( !wctx )
   {
      EMERROR( "WstCompositorCreate failed" );
      goto exit;
   }

   result= WstCompositorSetDisplayName( wctx, displayName );
   if ( !result )
   {
      EMERROR( "WstCompositorSetIsNested failed" );
      goto exit;
   }

   result= WstCompositorSetRendererModule( wctx, "libwesteros_render_nexus.so.0.0.0" );
   if ( result == false )
   {
      EMERROR( "WstCompositorSetRendererModule failed" );
      goto exit;
   }

   result= WstCompositorSetDefaultCursor( wctx, imgData, cursorWidth, cursorHeight, hotSpotX, hotSpotY );
   if ( result )
   {
      EMERROR( "WstCompositorSetDefaultCursor did not fail with non-running compositor" );
      goto exit;
   }

   result= WstCompositorStart( wctx );
   if ( result == false )
   {
      EMERROR( "WstCompositorStart failed" );
      goto exit;
   }

   wctx2= WstCompositorCreate();
   if ( !wctx2 )
   {
      EMERROR( "WstCompositorCreate failed" );
      goto exit;
   }

   result= WstCompositorSetDisplayName( wctx2, displayName2 );
   if ( !result )
   {
      EMERROR( "WstCompositorSetIsNested failed" );
      goto exit;
   }

   result= WstCompositorSetIsNested( wctx2, true );
   if ( !result )
   {
      EMERROR( "WstCompositorSetIsNested failed" );
      goto exit;
   }

   result= WstCompositorSetNestedDisplayName( wctx2, displayName );
   if ( !result )
   {
      EMERROR( "WstCompositorSetIsNested failed" );
      goto exit;
   }

   result= WstCompositorSetRendererModule( wctx2, "libwesteros_render_gl.so.0.0.0" );
   if ( result == false )
   {
      EMERROR( "WstCompositorSetRendererModule failed" );
      goto exit;
   }

   result= WstCompositorStart( wctx2 );
   if ( result == false )
   {
      EMERROR( "WstCompositorStart failed" );
      goto exit;
   }

   result= WstCompositorSetDefaultCursor( wctx2, imgData, cursorWidth, cursorHeight, hotSpotX, hotSpotY );
   if ( result )
   {
      EMERROR( "WstCompositorSetDefaultCursor did not fail with nested compositor" );
      goto exit;
   }

   WstCompositorDestroy( wctx2 );

   wctx2= WstCompositorCreate();
   if ( !wctx2 )
   {
      EMERROR( "WstCompositorCreate failed" );
      goto exit;
   }

   result= WstCompositorSetDisplayName( wctx2, displayName2 );
   if ( !result )
   {
      EMERROR( "WstCompositorSetIsNested failed" );
      goto exit;
   }

   result= WstCompositorSetRendererModule( wctx2, "libwesteros_render_gl.so.0.0.0" );
   if ( result == false )
   {
      EMERROR( "WstCompositorSetRendererModule failed" );
      goto exit;
   }

   result= WstCompositorSetIsRepeater( wctx2, true );
   if ( !result )
   {
      EMERROR( "WstCompositorSetIsRepeater failed" );
      goto exit;
   }

   result= WstCompositorSetNestedDisplayName( wctx2, displayName );
   if ( !result )
   {
      EMERROR( "WstCompositorSetIsNested failed" );
      goto exit;
   }

   result= WstCompositorStart( wctx2 );
   if ( result == false )
   {
      EMERROR( "WstCompositorStart failed" );
      goto exit;
   }

   result= WstCompositorSetDefaultCursor( wctx2, imgData, cursorWidth, cursorHeight, hotSpotX, hotSpotY );
   if ( result )
   {
      EMERROR( "WstCompositorSetDefaultCursor did not fail with repeating compositor" );
      goto exit;
   }

   WstCompositorDestroy( wctx2 );

   result= WstCompositorSetDefaultCursor( wctx, imgData, cursorWidth, cursorHeight, hotSpotX, hotSpotY );
   if ( !result )
   {
      EMERROR( "WstCompositorSetDefaultCursor failed" );
      goto exit;
   }

   // Allow default cursor to become active
   usleep( 100000 );

   result= WstCompositorSetDefaultCursor( wctx, imgData, cursorWidth, cursorHeight, hotSpotX, hotSpotY );
   if ( !result )
   {
      EMERROR( "WstCompositorSetDefaultCursor failed when called a second time" );
      goto exit;
   }

   WstCompositorDestroy( wctx );

   testResult= true;

exit:

   if ( imgData )
   {
      free( imgData );
   }

   return testResult;
}

static bool testCaseSocSinkBasicPipelineGfx( EMCTX *emctx )
{
   bool testResult= false;
   WstCompositor *wctx= 0;
   bool result;
   const char *value;
   int argc= 0;
   char **argv= 0;
   GstElement *pipeline= 0;
   GstElement *src= 0;
   GstElement *sink= 0;
   EMSimpleVideoDecoder *videoDecoder= 0;
   int stcChannelProxy;
   int videoPidChannelProxy;
   std::vector<WstRect> rects;
   float matrix[16];
   float alpha;
   bool needHolePunch;
   int hints;

   wctx= WstCompositorCreate();
   if ( !wctx )
   {
      EMERROR( "WstCompositorCreate failed" );
      goto exit;
   }

   result= WstCompositorSetRendererModule( wctx, "libwesteros_render_embedded.so.0.0.0" );
   if ( result == false )
   {
      EMERROR( "WstCompositorSetRenderedModule failed" );
      goto exit;
   }

   result= WstCompositorSetIsEmbedded( wctx, true );
   if ( result == false )
   {
      EMERROR( "WstCompositorSetIsEmbedded failed" );
      goto exit;
   }

   value= WstCompositorGetDisplayName( wctx );
   if ( value == 0 )
   {
      EMERROR( "WstCompositorGetDisplayName failed to return auto-generated name" );
      goto exit;
   }

   result= WstCompositorStart( wctx );
   if ( result == false )
   {
      EMERROR( "WstCompositorStart failed" );
      goto exit;
   }

   setenv( "WAYLAND_DISPLAY", value, true );

   videoDecoder= EMGetSimpleVideoDecoder( emctx, EM_TUNERID_MAIN );
   if ( !videoDecoder )
   {
      EMERROR("Failed to obtain test video decoder");
      goto exit;
   }

   EMSetStcChannel( emctx, (void*)&stcChannelProxy );
   EMSetVideoCodec( emctx, bvideo_codec_h264 );
   EMSetVideoPidChannel( emctx, (void*)&videoPidChannelProxy );
   EMSimpleVideoDecoderSetVideoSize( videoDecoder, 1920, 1080 );

   gst_init( &argc, &argv );

   pipeline= gst_pipeline_new("pipeline");
   if ( !pipeline )
   {
      EMERROR("Failed to create pipeline instance");
      goto exit;
   }

   src= createVideoSrc( emctx, videoDecoder );
   if ( !src )
   {
      EMERROR("Failed to create src instance");
      goto exit;
   }

   sink= gst_element_factory_make( "westerossink", "vsink" );
   if ( !sink )
   {
      EMERROR("Failed to create sink instance");
      goto exit;
   }

   gst_bin_add_many( GST_BIN(pipeline), src, sink, NULL );

   if ( gst_element_link( src, sink ) != TRUE )
   {
      EMERROR("Failed to link src and sink");
      goto exit;
   }

   gst_element_set_state( pipeline, GST_STATE_PLAYING );

   // Allow pipeline to run briefly.  Use animation hint to force gfx video path
   hints= WstHints_animating;
   for( int i= 0; i < 118; ++i )
   {
      usleep( 17000 );

      WstCompositorComposeEmbedded( wctx,
                                    0, // x
                                    0, // y
                                    WINDOW_WIDTH, // width
                                    WINDOW_HEIGHT, // height
                                    matrix,
                                    alpha,
                                    hints,
                                    &needHolePunch,
                                    rects );
   }

   gst_element_set_state( pipeline, GST_STATE_NULL );

   testResult= true;

   WstCompositorDestroy( wctx );

exit:

   unsetenv( "WAYLAND_DISPLAY" );

   return testResult;
}

static bool testCaseSocSinkVP9NonHDR( EMCTX *emctx )
{
   bool testResult= false;
   int argc= 0;
   char **argv= 0;
   GstElement *pipeline= 0;
   GstElement *src= 0;
   GstElement *sink= 0;
   EMSimpleVideoDecoder *videoDecoder= 0;
   int stcChannelProxy;
   int videoPidChannelProxy;
   int frameWidth= 3840;
   int frameHeight= 2160;
   int eotf= NEXUS_VideoEotf_eInvalid;
   float frameRate;
   int frameNumber;
   gint64 pos, posExpected;

   videoDecoder= EMGetSimpleVideoDecoder( emctx, EM_TUNERID_MAIN );
   if ( !videoDecoder )
   {
      EMERROR("Failed to obtain test video decoder");
      goto exit;
   }

   EMSetStcChannel( emctx, (void*)&stcChannelProxy );
   EMSetVideoCodec( emctx, bvideo_codec_vp9 );
   EMSetVideoPidChannel( emctx, (void*)&videoPidChannelProxy );
   EMSimpleVideoDecoderSetVideoSize( videoDecoder, frameWidth, frameHeight );

   gst_init( &argc, &argv );

   pipeline= gst_pipeline_new("pipeline");
   if ( !pipeline )
   {
      EMERROR("Failed to create pipeline instance");
      goto exit;
   }

   src= createVideoSrc( emctx, videoDecoder );
   if ( !src )
   {
      EMERROR("Failed to create src instance");
      goto exit;
   }

   sink= gst_element_factory_make( "westerossink", "vsink" );
   if ( !sink )
   {
      EMERROR("Failed to create sink instance");
      goto exit;
   }

   gst_bin_add_many( GST_BIN(pipeline), src, sink, NULL );

   if ( gst_element_link( src, sink ) != TRUE )
   {
      EMERROR("Failed to link src and sink");
      goto exit;
   }

   gst_element_set_state( pipeline, GST_STATE_PLAYING);

   // Allow pipeline to run briefly
   usleep( 1000000 );

   eotf= EMSimpleVideoDecoderGetHdrEotf( videoDecoder );
   if ( eotf != NEXUS_VideoEotf_eInvalid )
   {
      EMERROR("video decoder unexpectedly has HDR parameters");
      goto exit;
   }

   // Allow pipeline to run briefly
   usleep( 1000000 );

   eotf= EMSimpleVideoDecoderGetHdrEotf( videoDecoder );
   if ( eotf != NEXUS_VideoEotf_eInvalid )
   {
      EMERROR("video decoder unexpectedly has HDR parameters");
      goto exit;
   }

   for( int i= 0; i < 10; ++i )
   {
      usleep( INTERVAL_200_MS );

      frameRate= EMSimpleVideoDecoderGetFrameRate( videoDecoder );
      frameNumber= videoSrcGetFrameNumber( src );

      posExpected= (frameNumber/frameRate)*GST_SECOND;
      if ( !gst_element_query_position( pipeline, GST_FORMAT_TIME, &pos ) )
      {
         EMERROR("Failed to query position");
         goto exit;
      }
      g_print("%d position %" GST_TIME_FORMAT " expected %" GST_TIME_FORMAT "\n", i, GST_TIME_ARGS(pos), GST_TIME_ARGS(posExpected));
      if ( (pos < 0.9*posExpected) || (pos > 1.1*posExpected) )
      {
         EMERROR("Position out of range: expected %" GST_TIME_FORMAT " actual %" GST_TIME_FORMAT, GST_TIME_ARGS(posExpected), GST_TIME_ARGS(pos));
         goto exit;
      }
   }

   gst_element_set_state( pipeline, GST_STATE_NULL );

   testResult= true;

exit:
   if ( pipeline )
   {
      gst_object_unref( pipeline );
   }

   return testResult;
}

static bool testCaseSocSinkVP9HDRColorParameters( EMCTX *emctx )
{
   bool testResult= false;
   int argc= 0;
   char **argv= 0;
   GstElement *pipeline= 0;
   GstElement *src= 0;
   GstElement *sink= 0;
   EMSimpleVideoDecoder *videoDecoder= 0;
   int stcChannelProxy;
   int videoPidChannelProxy;
   int frameWidth= 3840;
   int frameHeight= 2160;
   int eotf= NEXUS_VideoEotf_eInvalid;
   float frameRate;
   int frameNumber;
   gint64 pos, posExpected;

   videoDecoder= EMGetSimpleVideoDecoder( emctx, EM_TUNERID_MAIN );
   if ( !videoDecoder )
   {
      EMERROR("Failed to obtain test video decoder");
      goto exit;
   }

   EMSetStcChannel( emctx, (void*)&stcChannelProxy );
   EMSetVideoCodec( emctx, bvideo_codec_unknown );
   EMSetVideoPidChannel( emctx, (void*)&videoPidChannelProxy );
   EMSimpleVideoDecoderSetVideoSize( videoDecoder, frameWidth, frameHeight );

   gst_init( &argc, &argv );

   pipeline= gst_pipeline_new("pipeline");
   if ( !pipeline )
   {
      EMERROR("Failed to create pipeline instance");
      goto exit;
   }

   src= createVideoSrc( emctx, videoDecoder );
   if ( !src )
   {
      EMERROR("Failed to create src instance");
      goto exit;
   }

   sink= gst_element_factory_make( "westerossink", "vsink" );
   if ( !sink )
   {
      EMERROR("Failed to create sink instance");
      goto exit;
   }

   gst_bin_add_many( GST_BIN(pipeline), src, sink, NULL );

   if ( gst_element_link( src, sink ) != TRUE )
   {
      EMERROR("Failed to link src and sink");
      goto exit;
   }

   gst_element_set_state( pipeline, GST_STATE_PLAYING);

   // Allow pipeline to run briefly
   usleep( 1000000 );

   eotf= EMSimpleVideoDecoderGetHdrEotf( videoDecoder );
   if ( eotf != NEXUS_VideoEotf_eInvalid )
   {
      EMERROR("video decoder unexpectedly has HDR parameters already");
      goto exit;
   }

   EMSetVideoCodec( emctx, bvideo_codec_vp9 );
   EMSimpleVideoDecoderSetColorimetry( videoDecoder, "2:6:13:7" );
   EMSimpleVideoDecoderSetMasteringMeta( videoDecoder, "0.677980:0.321980:0.245000:0.703000:0.137980:0.052000:0.312680:0.328980:1000.000000:0.000000" );
   EMSimpleVideoDecoderSetContentLight( videoDecoder, "1100:180" );

   // Set frame size which will cause a CAPS event to be emitted
   videoSrcSetFrameSize( src, frameWidth, frameHeight );

   // Allow pipeline to run briefly
   usleep( 1000000 );

   eotf= EMSimpleVideoDecoderGetHdrEotf( videoDecoder );
   if ( eotf == NEXUS_VideoEotf_eInvalid )
   {
      EMERROR("HDR parameters not set to video decoder");
      goto exit;
   }

   for( int i= 0; i < 10; ++i )
   {
      usleep( INTERVAL_200_MS );

      frameRate= EMSimpleVideoDecoderGetFrameRate( videoDecoder );
      frameNumber= videoSrcGetFrameNumber( src );

      posExpected= (frameNumber/frameRate)*GST_SECOND;
      if ( !gst_element_query_position( pipeline, GST_FORMAT_TIME, &pos ) )
      {
         EMERROR("Failed to query position");
         goto exit;
      }
      g_print("%d position %" GST_TIME_FORMAT " expected %" GST_TIME_FORMAT "\n", i, GST_TIME_ARGS(pos), GST_TIME_ARGS(posExpected));
      if ( (pos < 0.9*posExpected) || (pos > 1.1*posExpected) )
      {
         EMERROR("Position out of range: expected %" GST_TIME_FORMAT " actual %" GST_TIME_FORMAT, GST_TIME_ARGS(posExpected), GST_TIME_ARGS(pos));
         goto exit;
      }
   }

   gst_element_set_state( pipeline, GST_STATE_NULL );

   testResult= true;

exit:
   if ( pipeline )
   {
      gst_object_unref( pipeline );
   }

   return testResult;
}

namespace SocSinkGfxTransition
{
typedef struct _TestCtx
{
   TestEGLCtx eglCtx;
   WstGLCtx *glCtx;
   void *eglNativeWindow;
   int windowWidth;
   int windowHeight;
} TestCtx;

void textureCreated( EMCTX *ctx, void *userData, int bufferId )
{
   int *textureCount= (int*)userData;
   *textureCount= *textureCount + 1;
}

} //namespace SocSinkGfxTransition

static bool testCaseSocSinkGfxTransition( EMCTX *emctx )
{
   using namespace SocSinkGfxTransition;

   bool testResult= false;
   WstCompositor *wctx= 0;
   bool result;
   const char *value;
   int argc= 0;
   char **argv= 0;
   GstElement *pipeline= 0;
   GstElement *src= 0;
   GstElement *sink= 0;
   EMSimpleVideoDecoder *videoDecoder= 0;
   EMSurfaceClient *videoWindow= 0;
   int stcChannelProxy;
   int videoPidChannelProxy;
   std::vector<WstRect> rects;
   float matrix[16];
   float alpha;
   bool needHolePunch;
   int hints;
   EGLBoolean b;
   TestCtx testCtx;
   TestCtx *ctx= &testCtx;
   int vx, vy, vw, vh;
   int textureCount, texturesBeforeHidden;

   memset( &testCtx, 0, sizeof(TestCtx) );

   result= testSetupEGL( &ctx->eglCtx, 0 );
   if ( !result )
   {
      EMERROR("testSetupEGL failed");
      goto exit;
   }

   ctx->windowWidth= WINDOW_WIDTH;
   ctx->windowHeight= WINDOW_HEIGHT;

   ctx->glCtx= WstGLInit();
   if ( !ctx->glCtx )
   {
      EMERROR("Unable to create westeros-gl context");
      goto exit;
   }

   ctx->eglNativeWindow= WstGLCreateNativeWindow( ctx->glCtx, 0, 0, ctx->windowWidth, ctx->windowHeight );
   if ( !ctx->eglNativeWindow )
   {
      EMERROR("error: unable to create egl native window");
      goto exit;
   }
   printf("eglNativeWindow %p\n", ctx->eglNativeWindow);

   ctx->eglCtx.eglSurfaceWindow= eglCreateWindowSurface( ctx->eglCtx.eglDisplay,
                                                  ctx->eglCtx.eglConfig,
                                                  (EGLNativeWindowType)ctx->eglNativeWindow,
                                                  NULL );
   printf("eglCreateWindowSurface: eglSurfaceWindow %p\n", ctx->eglCtx.eglSurfaceWindow );

   b= eglMakeCurrent( ctx->eglCtx.eglDisplay, ctx->eglCtx.eglSurfaceWindow, ctx->eglCtx.eglSurfaceWindow, ctx->eglCtx.eglContext );
   if ( !b )
   {
      EMERROR("error: eglMakeCurrent failed: %X", eglGetError() );
      goto exit;
   }

   eglSwapInterval( ctx->eglCtx.eglDisplay, 1 );

   wctx= WstCompositorCreate();
   if ( !wctx )
   {
      EMERROR( "WstCompositorCreate failed" );
      goto exit;
   }

   result= WstCompositorSetRendererModule( wctx, "libwesteros_render_embedded.so.0.0.0" );
   if ( result == false )
   {
      EMERROR( "WstCompositorSetRenderedModule failed" );
      goto exit;
   }

   result= WstCompositorSetIsEmbedded( wctx, true );
   if ( result == false )
   {
      EMERROR( "WstCompositorSetIsEmbedded failed" );
      goto exit;
   }

   value= WstCompositorGetDisplayName( wctx );
   if ( value == 0 )
   {
      EMERROR( "WstCompositorGetDisplayName failed to return auto-generated name" );
      goto exit;
   }

   result= WstCompositorStart( wctx );
   if ( result == false )
   {
      EMERROR( "WstCompositorStart failed" );
      goto exit;
   }

   setenv( "WAYLAND_DISPLAY", value, true );

   videoDecoder= EMGetSimpleVideoDecoder( emctx, EM_TUNERID_MAIN );
   if ( !videoDecoder )
   {
      EMERROR("Failed to obtain test video decoder");
      goto exit;
   }

   EMSetStcChannel( emctx, (void*)&stcChannelProxy );
   EMSetVideoCodec( emctx, bvideo_codec_h264 );
   EMSetVideoPidChannel( emctx, (void*)&videoPidChannelProxy );
   EMSimpleVideoDecoderSetVideoSize( videoDecoder, 1920, 1080 );

   memset( &matrix, 0, sizeof(matrix) );
   matrix[0]= matrix[5]= matrix[10]= matrix[15]= 1.0;

   gst_init( &argc, &argv );

   pipeline= gst_pipeline_new("pipeline");
   if ( !pipeline )
   {
      EMERROR("Failed to create pipeline instance");
      goto exit;
   }

   src= createVideoSrc( emctx, videoDecoder );
   if ( !src )
   {
      EMERROR("Failed to create src instance");
      goto exit;
   }

   sink= gst_element_factory_make( "westerossink", "vsink" );
   if ( !sink )
   {
      EMERROR("Failed to create sink instance");
      goto exit;
   }

   gst_bin_add_many( GST_BIN(pipeline), src, sink, NULL );

   if ( gst_element_link( src, sink ) != TRUE )
   {
      EMERROR("Failed to link src and sink");
      goto exit;
   }

   gst_element_set_state( pipeline, GST_STATE_PLAYING );

   // Allow pipeline to run briefly.
   hints= WstHints_noRotation;
   for( int i= 0; i < 2; ++i )
   {
      usleep( 17000 );

      rects.clear();
      WstCompositorComposeEmbedded( wctx,
                                    0, // x
                                    0, // y
                                    WINDOW_WIDTH, // width
                                    WINDOW_HEIGHT, // height
                                    matrix,
                                    alpha,
                                    hints,
                                    &needHolePunch,
                                    rects );

      eglSwapBuffers(ctx->eglCtx.eglDisplay, ctx->eglCtx.eglSurfaceWindow);
   }


   videoWindow= EMGetVideoWindow( emctx, EM_TUNERID_MAIN );

   EMSetTextureCreatedCallback( emctx, textureCreated, &textureCount );

   // Switch to graphics path
   textureCount= 0;
   texturesBeforeHidden= -1;
   hints= WstHints_animating;
   for( int i= 0; i < 20; ++i )
   {
      usleep( 17000 );

      rects.clear();
      WstCompositorComposeEmbedded( wctx,
                                    0, // x
                                    0, // y
                                    WINDOW_WIDTH, // width
                                    WINDOW_HEIGHT, // height
                                    matrix,
                                    alpha,
                                    hints,
                                    &needHolePunch,
                                    rects );

      eglSwapBuffers(ctx->eglCtx.eglDisplay, ctx->eglCtx.eglSurfaceWindow);

      EMSurfaceClientGetPosition( videoWindow, &vx, &vy, &vw, &vh );
      if ( (vy == -vh) && (texturesBeforeHidden == -1) )
      {
         texturesBeforeHidden= textureCount;
      }
   }

   gst_element_set_state( pipeline, GST_STATE_NULL );

   if ( texturesBeforeHidden < 1 )
   {
      EMERROR("Video hidden during animation too soon: texturesBeforeHidden: %d", texturesBeforeHidden);
      goto exit;
   }

   testResult= true;

exit:

   if ( wctx )
   {
      WstCompositorDestroy( wctx );
   }

   if ( ctx->eglCtx.eglSurfaceWindow )
   {
      eglDestroySurface( ctx->eglCtx.eglDisplay, ctx->eglCtx.eglSurfaceWindow );
      ctx->eglCtx.eglSurfaceWindow= EGL_NO_SURFACE;
   }

   if ( ctx->eglNativeWindow )
   {
      WstGLDestroyNativeWindow( ctx->glCtx, ctx->eglNativeWindow );
      ctx->eglNativeWindow= 0;
   }

   if ( ctx->glCtx )
   {
      WstGLTerm( ctx->glCtx );
      ctx->glCtx= 0;
   }

   testTermEGL( &ctx->eglCtx );

   unsetenv( "WAYLAND_DISPLAY" );

   return testResult;
}

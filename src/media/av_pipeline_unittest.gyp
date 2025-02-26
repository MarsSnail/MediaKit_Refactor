{
  'target_defaults': {
    'conditions': [
      ['OS=="linux"', {
        'defines': [
          '__unix__',
          '_LINUX'
        ],
        'cflags': [
          '-Wall'
        ]
      }],
      ['OS=="win"', {
        'defines': [
          'WIN32'
        ],
        'msvs_configuration_attributes': {
          'CharacterSet': '1'
        },
        'msvs_settings': {
          'VCCLCompilerTool': {
            'WarningLevel': '4',
            'Detect64BitPortabilityProblems': 'true'
          }
        }
      }],
      ['OS=="mac"', {
        'defines': [
          '__unix__',
          '_MACOS'
        ],
        'cflags': [
          '-Wall'
        ]
      }]
    ],

    'configurations': {
      'Debug': {
        'defines': [
          '_DEBUG'
        ],
        'conditions': [
          ['OS=="linux" or OS=="freebsd" or OS=="openbsd"', {
            'cflags': [
              '-g3',
              '-gstabs+',
              '-O0'
            ]
          }],
          ['OS=="win"', {
            'msvs_settings': {
              'VCCLCompilerTool': {
                'Optimization': '0',
                'MinimalRebuild': 'true',
                'BasicRuntimeChecks': '3',
                'DebugInformationFormat': '4',
                'RuntimeLibrary': '3'  # /MDd
              },
              'VCLinkerTool': {
                'GenerateDebugInformation': 'true',
                'LinkIncremental': '2'
              }
            }
          }],
          ['OS=="mac"', {
            'xcode_settings': {
              'GCC_GENERATE_DEBUGGING_SYMBOLS': 'YES',
            }
          }]
        ]
      },

      'Release': {
        'conditions': [
          ['OS=="linux" or OS=="freebsd" or OS=="openbsd"', {
            'cflags': [
              '-O3'
            ]
          }],
          ['OS=="win"', {
            'msvs_settings': {
              'VCCLCompilerTool': {
                'Optimization': '2',
                'RuntimeLibrary': '2'  # /MD
              }
            }
          }],
          ['OS=="mac"', {
            'xcode_settings': {
              'GCC_GENERATE_DEBUGGING_SYMBOLS': 'NO',
              'GCC_OPTIMIZATION_LEVEL': '3',

              # -fstrict-aliasing. Mainline gcc enables
              # this at -O2 and above, but Apple gcc does
              # not unless it is specified explicitly.
              'GCC_STRICT_ALIASING': 'YES'
            }
          }]
        ]
      }
    }
  },

  'targets': [
    {
      'target_name': 'av_pipeline_unittest',
      'type': 'executable',  # executable, <(library)
      # 'dependencies': [
      # ],
      # 'defines': [
      # ],
      'include_dirs': [
            '../../src',
            '../third_party/boost/include',
            '../third_party/ffmpeg/include',
            '../third_party/freeglut/include',
            '../third_party/sdl/include',
            '../third_party/curl/include',
            '/opt/X11/include'
      ],
      'conditions': [
      ],
      'sources': [
        'av_pipeline.h',
        'av_pipeline.cpp',
        'base/audio_parameters.h',
        'base/video_decoder_config.h',
        'base/video_decoder_config.cpp',
        'base/audio_decoder_config.h',
        'base/audio_decoder_config.cpp',
        'base/channel_layout.h',
        'base/sample_format.h',
        'base/time_source.h',
        'base/audio_frame.h',
        'base/audio_frame.cpp',
        'base/video_frame.h',
        'base/video_frame.cpp',
        'base/wall_clock_time_source_impl.h',
        'base/wall_clock_time_source_impl.cpp',
        #'../third_party/glew/GL/glew.h',
        #'../third_party/glew/glew.c',
        'demuxer/demuxer.h',
        'demuxer/demuxer.cpp',
        'demuxer/demuxer_stream.h',
        'demuxer/demuxer_stream.cpp',
        'demuxer/demuxer_stream_provider.h',
        'demuxer/ffmpeg_demuxer_stream.h',
        'demuxer/ffmpeg_demuxer_stream.cpp',
        'demuxer/ffmpeg_demuxer.h',
        'demuxer/ffmpeg_demuxer.cpp',
        'decoder/audio_decoder.h',
        'decoder/ffmpeg_audio_decoder.h',
        'decoder/ffmpeg_audio_decoder.cpp',
        'decoder/audio_frame_stream.h',
        'decoder/audio_frame_stream.cpp',
        'decoder/ffmpeg_video_decoder.h',
        'decoder/ffmpeg_video_decoder.cpp',
        'decoder/video_decoder.h',
        'decoder/video_frame_stream.h',
        'decoder/video_frame_stream.cpp',
        'renderer/audio_renderer.h',
        'renderer/audio_renderer_impl.h',
        'renderer/audio_renderer_impl.cpp',
        'renderer/audio_renderer_sink.h',
        'renderer/audio_renderer_sink.cpp',
        'renderer/sdl_audio_renderer_sink.h',
        'renderer/sdl_audio_renderer_sink.cpp',
        'renderer/renderer.h',
        'renderer/renderer_impl.h',
        'renderer/renderer_impl.cpp',
        'renderer/video_renderer.h',
        'renderer/video_renderer_impl.h',
        'renderer/video_renderer_impl.cpp',
        'ffmpeg/ffmpeg_common.h',
        'ffmpeg/ffmpeg_common.cpp',
        '../net/file_stream_provider.h',
        '../net/file_stream_provider.cpp',
        '../net/io_channel.h',
        '../net/io_channel.cpp',
        '../net/url.h',
        '../net/url.cpp',
        '../net/curl/curl_adapter.h',
        '../net/curl/curl_adapter.cpp',
        '../base/CommonDef.h',
        '../base/ImageType.cpp',
        '../base/ImageType.h',
        '../base/SnailConfig.h',
        '../base/SnailException.h',
        '../base/timer/ClockTime.h',
        '../base/timer/ClockTime.cpp',
        '../base/timer/SnailSleep.h',
        '../base/timer/SystemClock.h',
        '../base/timer/SystemClock.cpp',
        '../base/timer/VirtualClock.h',
        '../base/timer/wall_clock_timer.h',
        '../base/timer/wall_clock_timer.cpp',
        '../base/timer/scheduler_timer.h',
        '../base/timer/scheduler_timer.cpp',
        'av_pipeline_unittest.cpp'
        ],
      'conditions': [
         ['OS=="linux" or OS=="freebsd" or OS=="openbsd"', {
           'ldflags': [
             '-'
           ]
         }],
         ['OS=="win"', {
           'msvs_settings': {
             'VCLinkerTool': {
               'ImageHasSafeExceptionHandlers' : 'false',
               'AdditionalLibraryDirectories': [
                  '$(OutDir)\\lib',
                  '../third_party/boost/lib/win',
                  '../third_party/freeglut/lib/win',
                  '../third_party/glew/lib/<(CONFIGURATION_NAME)/win32',
                  '../third_party/curl/lib/<(CONFIGURATION_NAME)/win32',
                  '../third_party/ffmpeg/lib/win',
                  '../third_party/sdl/lib/win/x86'
                  ],
               'AdditionalDependencies': [
                  'libboost_thread-vc120-mt-gd-1_56.lib',
                  'glew32sd.lib',
				  'avcodec.lib',
                  'avformat.lib',
                  'avcodec.lib',
                  'avutil.lib',
				  'sdl.lib',
                  'swresample.lib',
                  'swscale.lib',
                  'ws2_32.lib',
                  'wldap32.lib',
                  'libcurld.lib',
                  'kernel32.lib',
                  'user32.lib',
                  'gdi32.lib',
                  'winspool.lib',
                  'comdlg32.lib',
                  'advapi32.lib',
                  'shell32.lib'
                ],
             }
           },
          
          'defines': [
              'BUILDING_LIBCURL',
              'DEBUGBUILD',
              'CURL_STATICLIB',
              'WIN32_LEAN_AND_MEAN',
              'GLEW_STATIC',
			  'WIN32'
          ],
         }],
          ['OS=="mac"', {
            'mac_framework_dirs':[
              '../../depend/mac/framework',
              '$(SDKROOT)/System/Library/Frameworks/',
            ],
            'xcode_settings':{
            },
            'library_dirs':[
            '../../depend/mac/lib',
            '../../depend/mac/lib/boost',
            ],
           'libraries':
           [
            'SDL2.framework',
            'CoreFoundation.framework',
            'AVFoundation.framework',
            'AudioToolbox.framework',
            'CoreVideo.framework',
            'VideoDecodeAcceleration.framework',
            'OpenGL.framework',
            'libavcodec.a',
            'libavdevice.a',
            'libavfilter.a',
            'libavformat.a',
            'libavutil.a',
            'libz.a',
            'libiconv.a',
            'libswresample.a',
            'libswscale.a',
            'libcurl.a',
            'libglut.a',
            'libbz2.a',
            'libboost_thread.a',
            'libboost_system.a',
           ],
           'include_dirs':[
            '../../depend/mac/include',
           ],
         }]
       ]
    }
  ]
}

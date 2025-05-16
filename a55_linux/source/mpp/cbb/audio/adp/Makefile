# include source/Makefile.param
ifeq ($(PARAM_FILE), )
    PARAM_FILE:=../../../../Makefile.param
    include $(PARAM_FILE)
endif

ifeq ($(CBB_PARAM_FILE), )
    CBB_PARAM_FILE:=$(SOURCE_PATH)/mpp/cbb/Makefile.param
    include $(CBB_PARAM_FILE)
endif

LIB_NAME = ss_mpi_audio_adp

AUDIO_ROOT ?= $(SOURCE_PATH)/mpp/cbb/audio

MPI_CFLAGS += -I$(AUDIO_ROOT)/adp/include
MPI_CFLAGS += -I$(AUDIO_ROOT)/adp/src

ifeq ($(MPP_BUILD), y)
MPI_CFLAGS += -I$(AUDIO_ROOT)/mpi/audio
MPI_CFLAGS += -I$(AUDIO_ROOT)/mpi/audio
MPI_CFLAGS += -I$(AUDIO_PATH)/aacenc/include
MPI_CFLAGS += -I$(AUDIO_PATH)/aacdec/include
ifeq ($(CONFIG_OT_AENC_MP3_SUPPORT), y)
MPI_CFLAGS += -I$(AUDIO_PATH)/mp3enc/include
endif
ifeq ($(CONFIG_OT_ADEC_MP3_SUPPORT), y)
MPI_CFLAGS += -I$(AUDIO_PATH)/mp3dec/include
endif
ifneq ($(findstring y, $(CONFIG_OT_AENC_OPUS_SUPPORT) $(CONFIG_OT_ADEC_OPUS_SUPPORT)), )
MPI_CFLAGS += -I$(AUDIO_PATH)/opus/include
endif
endif

MPI_SRC_DIR := $(AUDIO_ROOT)/adp/src
MPI_SRC_C := ot_audio_dl_adp.c

ifneq ($(findstring y, $(CONFIG_OT_AENC_AAC_SUPPORT) $(CONFIG_OT_ADEC_AAC_SUPPORT)), )
MPI_SRC_C += ot_audio_aac_adp.c
endif
ifneq ($(findstring y, $(CONFIG_OT_AENC_MP3_SUPPORT) $(CONFIG_OT_ADEC_MP3_SUPPORT)), )
MPI_SRC_C += ot_audio_mp3_adp.c
endif
ifneq ($(findstring y, $(CONFIG_OT_AENC_OPUS_SUPPORT) $(CONFIG_OT_ADEC_OPUS_SUPPORT)), )
MPI_SRC_C += ot_audio_opus_adp.c
endif

INC_FILE += $(AUDIO_ROOT)/adp/include/ot_audio_dl_adp.h
ifneq ($(findstring y, $(CONFIG_OT_AENC_AAC_SUPPORT) $(CONFIG_OT_ADEC_AAC_SUPPORT)), )
INC_FILE += $(AUDIO_ROOT)/adp/include/ot_audio_aac_adp.h
endif
ifneq ($(findstring y, $(CONFIG_OT_AENC_MP3_SUPPORT) $(CONFIG_OT_ADEC_MP3_SUPPORT)), )
INC_FILE += $(AUDIO_ROOT)/adp/include/ot_audio_mp3_adp.h
endif
ifneq ($(findstring y, $(CONFIG_OT_AENC_OPUS_SUPPORT) $(CONFIG_OT_ADEC_OPUS_SUPPORT)), )
INC_FILE += $(AUDIO_ROOT)/adp/include/ot_audio_opus_adp.h
endif

##### select library type for aac #####
AAC_LIB_TYPE ?= LIB_TYPE_STATIC
#AAC_LIB_TYPE ?= LIB_TYPE_SHARE
ifeq ($(AAC_LIB_TYPE), LIB_TYPE_STATIC)
	MPI_CFLAGS += -DOT_AAC_USE_STATIC_MODULE_REGISTER
	MPI_CFLAGS += -DOT_AAC_HAVE_SBR_LIB
endif

##### dynamic library use type for mp3 ######
# MPI_CFLAGS += -DOT_MP3_USE_DYNAMIC_LOAD # default close

include $(MAKE_MPI_FILE)

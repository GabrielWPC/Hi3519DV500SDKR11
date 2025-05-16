ifeq ($(PARAM_FILE), )
     PARAM_FILE := ../../Makefile.param
     include $(PARAM_FILE)
endif

ifeq ($(CONFIG_OT_GDC_FISHEYE_SUPPORT), y)
     LIBS += $(REL_LIB)/libss_mpi_fisheye_calibrate.a
     SRC := $(wildcard *.c)
else
     ALLSRC := $(wildcard *.c)
     SRC := $(filter-out fisheye_calibration.c, $(ALLSRC))
endif

ifeq ($(CONFIG_OT_AVS_SUPPORT), )
     SRC := $(filter-out avs_chn_dump.c avs_src_dump.c, $(SRC))
endif

ifeq ($(CONFIG_OT_VI_ALL_SUPPORT), y)
     SRC := $(filter-out vi_dump.c, $(SRC))
else ifeq ($(CONFIG_OT_VI_XVR_SUPPORT), y)
     SRC := $(filter-out vi_pipe_dump.c vi_chn_dump.c, $(SRC))
else
     SRC := $(filter-out vi_pipe_dump.c vi_chn_dump.c vi_dump.c, $(SRC))
endif

ifeq ($(CONFIG_MIPI_TX), )
    SRC := $(filter-out mipitx_read.c mipitx_write.c, $(SRC))
endif


ifeq ($(CONFIG_OT_MCF_SUPPORT), )
	SRC := $(filter-out mcf_chn_dump.c mcf_grp_dump.c, $(SRC))
endif

ifeq ($(CONFIG_OT_AIISP_SUPPORT), )
	SRC := $(filter-out aiisp_dump.c, $(SRC))
endif

ifeq ($(CONFIG_OT_MCF_SUPPORT), y)
	LIBS += $(REL_LIB)/libss_mpi_mcf.a
endif

ifeq ($(CONFIG_OT_AIISP_SUPPORT), y)
	CFLAGS += -I$(SDK_PATH)/source/mpp/cbb/vb/include/inner
ifeq ($(CONFIG_OT_AIBNR_SUPPORT), y)
	CFLAGS += -DOT_AIBNR_SUPPORT
	LIBS += $(REL_LIB)/libss_mpi_aibnr.a
endif
ifeq ($(CONFIG_OT_AI3DNR_SUPPORT), y)
	CFLAGS += -DOT_AI3DNR_SUPPORT
	LIBS += $(REL_LIB)/libss_mpi_ai3dnr.a
endif
ifeq ($(CONFIG_OT_AIDRC_SUPPORT), y)
        CFLAGS += -DOT_AIDRC_SUPPORT
        LIBS += $(REL_LIB)/libss_mpi_aidrc.a
endif
ifeq ($(CONFIG_OT_SVP_SUPPORT), y)
	LIBS += $(REL_LIB)/libsvp_acl.a
	LIBS += $(REL_LIB)/libss_mpi_km.a
	LIBS += $(REL_LIB)/libprotobuf-c.a
endif
endif

ifeq ($(CONFIG_OT_DIS_SUPPORT), )
	SRC := $(filter-out dis_dump.c, $(SRC))
endif

OBJ := $(SRC:%.c=%.o)

INC_FLAGS := -I$(REL_INC)
INC_FLAGS += -I$(REL_INC)
CFLAGS += -Wall -g $(INC_FLAGS)
CFLAGS += $(LIBS_CFLAGS)
CFLAGS += $(LIBS_EXEC_CFLAGS)

LIBS += $(REL_LIB)/libss_mpi.a
LIBS += $(REL_LIB)/libss_mpi_sysbind.a
LIBS += $(REL_LIB)/libss_mpi_sysmem.a
LIBS += $(REL_LIB)/libss_mpi_motionfusion.a
LIBS += $(REL_LIB)/libsecurec.a

ifeq ($(CONFIG_OT_ISP_SUPPORT), y)
LIBS += $(REL_LIB)/libss_mpi_ae.a
LIBS += $(REL_LIB)/libss_mpi_isp.a
LIBS += $(REL_LIB)/libot_mpi_isp.a
LIBS += $(REL_LIB)/libss_mpi_awb.a
LIBS += $(REL_LIB)/libdehaze.a
LIBS += $(REL_LIB)/libextend_stats.a
LIBS += $(REL_LIB)/libdrc.a
LIBS += $(REL_LIB)/libldci.a
LIBS += $(REL_LIB)/libbnr.a
LIBS += $(REL_LIB)/libcalcflicker.a
LIBS += $(REL_LIB)/libir_auto.a
LIBS += $(REL_LIB)/libacs.a
ifeq ($(CONFIG_OT_ISP_DUMP_DEBUG_SUPPORT), y)
LIBS += $(REL_LIB)/libdump_dbg.a
endif
endif

ifeq ($(CONFIG_OT_AUDIO_SUPPORT), y)
LIBS += $(REL_LIB)/libss_mpi_audio.a
LIBS += $(REL_LIB)/libvoice_engine.a
LIBS += $(REL_LIB)/libupvqe.a
LIBS += $(REL_LIB)/libdnvqe.a
endif

LIBS += $(LIBS_LD_CFLAGS)
CFLAGS += $(LIBS_CFLAGS)
ifneq ($(CONFIG_LLVM), y)
CFLAGS += -lpthread -ldl -lstdc++ -lm
endif

include $(PWD)/Make.$(OSTYPE)

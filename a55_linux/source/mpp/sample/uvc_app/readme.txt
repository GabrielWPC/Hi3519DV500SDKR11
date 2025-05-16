Hardware Tip
    USB_VBUS multiplex with GPIO10_3(VO_RESET, BT1120 and BT656), remove R4812 and connect R4811 on demo board!

Config pin_mux of USB
    The pin_mux should be config to the USB function. As follows：
    1 update the file: smp/a55_linux/source/interdrv/sysconfig/pin_mux.h
        #define USB_EN 1
    2 enter the directory and compile:
        cd smp/a55_linux/source/interdrv;
        make clean; make

Compile:
	only compile uvc default, if need uac, please change the value of "UAC_COMPILE" into "y" in Makefile.
	$ make clean
	$ make


There are 4 data input mode to select:
1. OT_UVC_MPP_BIND_UVC: for venc stream or yuv frame, VENC bind UVC or VPSS/VI bind UVC, stream data flows in the kernel space;
2. OT_UVC_SEND_VENC_STREAM: user get VENC stream from venc channel, save or send out by network, then send venc stream to UVC by ss_mpi_uvc_send_stream();
3. OT_UVC_SEND_YUV_FRAME: user get YUV frame from VI/VPSS, then send it to UVC by ss_mpi_uvc_send_frame();
4. OT_UVC_SEND_USER_STREAM: user have private buffer that contains encoded stream, send it to UVC by ss_mpi_uvc_send_user_stream(), this is not for YUV frame, we can use VI/VPSS bind UVC for YUV frame.

uvc_media.c has a global variable: g_data_input_mode, it decides the send mode.If g_data_input_mode is changed, then rebuild the uvc_app sample.


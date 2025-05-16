This tool is used to calibrate dynamic blc offset and dynamic blc cali black level.
This version does not support liteos.

Calibration process of black level compensation parameters:
Step 1: Modify the sensor driver (os08a20_cmos_ex.h). Change OT_ISP_BLACK_LEVEL_MODE_DYNAMIC to dynamic BLC mode.

Step 2: Close the aperture of the device or use the lens cover to block the lens input to ensure that no light enters the device.

Step 3: Run the calibration tool dynamic_blc_online_cali(mpp/sample/dynamic_blc_online_cali # ./sample_dynamic_blc_online_cali 0 30 7 100 100 100 100).

usage: ./sample_dynamic_blc_online_cali [vi_pipe] [frame_cnt] [iso_num] [light_stt_area.x] [light_stt_area.y] [light_stt_area.width] [light_stt_area.height]

vi_pipe:
   0:vi_pipe0 ~~ 3:vi_pipe3
frame_cnt:
   frame_cnt value to be used to calculate black level(range:(0,30])
iso_num:
   iso_num value to be used to set again(range:(0,16])
light_stt_area.x:
        X coordinates of the light statistical region(range:[0, image_width])
light_stt_area.y:
        Y coordinates of the light statistical region(range:[ob_height, image_height])
light_stt_area.width:
        width of the light statistical region(range:[0,image_width - light_stt_area.x])
light_stt_area.height:
        height of the light statistical region(range:[0, image_height - light_stt_area.y])
e.g : ./sample_dynamic_blc_online_cali 0 30 8 100 100 100 100

Notes:

Calibration area can be manually set with the calibration sample tool. The larger the statistical area, the longer the time. The more iso gears selected, the longer the time.

After the calibration is completed, the SAMPLE will automatically import the calibration data without manual import.

The current calibration tool is only adapted to IMX347, OS08A20. To calibrate other sensors, set the statistical process for the gain value corresponding to the sensor in the set_gain_acor_sensor function.

1.use thermo t3  load command:./load3519dv500 -a -sensor1 t3
2.modify Makefile.param in sample folder, set SENSOR1_TYPE ?= GST_412C_SLAVE_THERMO_T3_384_288_30FPS_14BIT
3.complile thermo sample, exec ./sample_thermo 0

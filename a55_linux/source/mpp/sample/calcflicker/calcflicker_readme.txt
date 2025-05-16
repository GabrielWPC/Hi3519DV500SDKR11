1. Before using this sample, you should modify  smp/a55_linux/source/ko/load3519dv500
1.1 modify the insert_ko() function "insmod ot_vgs" into the following:
    insmod ot_vgs.ko g_max_vgs_task=200 g_max_vgs_node=200

1. 此sample需要配置管脚复用，加载sys_config.ko时，需要注意和vo_intf进行区分，需要使用ir_auto时（加载ko默认打开），vo_intf需要配置为非RGB格式，加载ko时可以使用如下参数：
./load3519dv500 -i -vo_intf bt1120
参数-ir_auto默认配置为1，如果不需要使用，可以关闭，例如：
./load3519dv500 -i -vo_intf bt1120 -ir_auto 0
2. 本sample会启动ISP，VPSS等业务，无需额外启动，只需要按照sample提示说明使用即可。

详情可以通过以下命令查看：
linux:./sample_ir_auto -h



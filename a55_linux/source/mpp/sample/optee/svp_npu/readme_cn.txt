用例使用说明:

步骤1: 模型加密和签名
    原始模型文件在sample/svp/svp_npu/data/model目录下
    使用密钥生成工具KDF、加密工具SecureTool进行配置、加密和签名
    将工具生成的文件放置到host/data/model目录下，保持其文件名不变

步骤2: 实现公钥获取方法
    当前TA_SetPubKey为空函数，使用前需要实现以上函数，并调用svp_npu_set_pub_key接口配置公钥，不然无法使用验签功能

步骤3: 配置LD_LIBRARY_PATH
    执行export LD_LIBRARY_PATH=xxx/mpp/out/lib:$LD_LIBRARY_PATH，配置LD_LIBRARY_PATH
    xxx是sdk软件包路径

步骤4: 拷贝密钥
    将实际使用optee镜像对应的default.pem密钥文件放置到smp/a55_optee/keys路径下，以与之匹配的ta文件

步骤5: 编译用例
    执行make -j编译用例

步骤6: 放置ta到板端flash
    执行cp ta/426e8f6f-ec74-427b-9424-0dd8f3f33706.ta /lib/optee_armtz/
    /lib/optee_armtz为固定的ta存放路径

步骤7: 后台运行teec_supplicant
    执行teec_supplicant & 后台启动ta守护进程

步骤8: 运行用例
    执行./sample_svp_npu_main <index> 运行用例

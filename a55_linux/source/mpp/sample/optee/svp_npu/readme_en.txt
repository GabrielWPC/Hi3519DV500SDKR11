sample usage:

Step 1: Encrypt and sign model
    model file is in sample/svp/svp_npu/data/model
    using KDF, SecureTool to config, encrypt and sign it
    move it to host/data/model, keep it name as before

Step 2: Implement function to set pub key
    TA_SetPubKey is empty function
    please implement it and using svp_npu_set_pub_key to set put key before running, or sign function is not available

Step 3: set LD_LIBRARY_PATH
    export LD_LIBRARY_PATH=xxx/mpp/out/lib:$LD_LIBRARY_PATH
    xxx is sdk package path.

Step 4: copy keys
    please copy the default.pem of your OPTEE-OS to smp/a55_optee/keys, or ta is not suitable for your board

Step 5: compile sample code
    make -j;

Step 6: put ta to rootfs
    cp ta/426e8f6f-ec74-427b-9424-0dd8f3f33706.ta /lib/optee_armtz/

Step 7: run teec_supplicant in background
    teec_supplicant &

Step 8: run sample
    ./sample_svp_npu_main <index>

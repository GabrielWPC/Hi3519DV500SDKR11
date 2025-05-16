sample usage:

Notice:
You should copy the default.pem from optee-os-xxx/keys to smp/a55_optee/keys
if you want ta run on the board successfully.

Step 1: compile sample code
    make -j;

Step 2: put ta to rootfs
    cp ta/*.ta /lib/optee_armtz

Step 3: run tee-supplicant in background
    ./tee-supplicant &

Step 4: run sample
    ./sample_cipher <index>
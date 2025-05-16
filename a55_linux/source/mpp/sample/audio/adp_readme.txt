This document briefly describes how to use audio_adp lib.

1.select lib type and compile audio_adp
    1) Go to mpp/cbb/audio/adp folder.
    2) Select the AAC_LIB_TYPE option in mpp/cbb/audio/adp/Makefile.
    3) Compile audio_adp lib if option is changed.

    Note: The AAC supports static library registration and dynamic library loading.
    Examples:
        cd ./mpp/cbb/audio/adp;
        vi Makefile;
        make clean;
        make

2.select lib type and compile sample_audio
    1) Go to mpp/sample folder.
    2) Select the AUDIO_MODULE_LIB_TYPE option in mpp/sample/Makefile.param, it should be match AAC_LIB_TYPE.
    3) Compile sample_audio if option is changed.

    Examples:
        cd ./mpp/sample;
        vi Makefile.param;
        cd ./audio;
        make clean;
        make

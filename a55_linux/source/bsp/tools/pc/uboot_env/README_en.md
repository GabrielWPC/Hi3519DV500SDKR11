[toc]

# UBOOT_ENV USAGE

## 1. Introdution
There are two main functions of this component: 
a. to convert the U-boot environment variable (env) from text to a binary image that can be directly burned.
b. to create the partition table of ToolPlatform.exe based on the bootargs partition information in the env file.

## 2. How to Use
### 2.1 Build
Execute "make" to build targets:
```sh
make ENVTXT=<env text path> all

# Specify the evn size by argument "SIZE" (256KB as defalut)
make ENVTXT=<env text path> SIZE=<size> all
```

Target will be generated in "uboot_env/"：
- env.bin: the binary U-Boot environment variable that can be directly burned
- burn_table.xml：the partition table of ToolPlatform.exe

### 2.2 Clean 
Execute "make clean" to clean the generated files：
```sh
make clean
```

## 3. Examples
This section takes "env_text/hi3519dv500/emmc_env.txt" as an example.

### 3.1 Generate env.bin and Partition Table
```sh
make ENVTXT=env_text/hi3519dv500/emmc_env.txt all
```

### 3.2 Generate env.bin only
- Generate 256KB (default) env.bin.
```sh
make ENVTXT=env_text/hi3519dv500/emmc_env.txt env
```
- Generate 1MB env.bin.
```sh
make ENVTXT=env_text/hi3519dv500/emmc_env.txt SIZE=0x100000 env
```

### 3.3 Generate Partition Table only
```sh
make ENVTXT=env_text/hi3519dv500/emmc_env.txt burnxml
```

## 4. Directory Tree Description
```
uboot_env/
├── Makefile ----------- the top level Makefile
├── REAME_en.md ----------- this document
├── env_text ----------- examples of environment variable text
└── scripts
    └── env2burn.py ---- the scrpit to generate partition table of ToolPlatform.exe
```

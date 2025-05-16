[toc]

# uboot_env 使用说明

## 一、功能介绍
1、将 u-boot 环境变量（env）从文本转换成可直接烧写的二进制镜像；
2、根据 env 中的 bootargs 的分区信息，创建 ToolPlatform 分区表。

## 二、使用方法
### 2.1 编译
执行 make 命令生成目标文件：
```sh
make ENVTXT=<env 文本路径> all

# 指定环境变量 SIZE（不指定 SIZE 默认 256KB）
make ENVTXT=<env 文本路径> SIZE=<size> all
```

在 uboot_env/ 目录下生成 2 个目标文件：
- env.bin：二进制 U-Boot 环境变量，可直接烧写到介质
- burn_table.xml：ToolPlatform 分区表文件。

### 2.2 清除
执行 clean 清除生成的目标文件：
```sh
make clean
```

## 三、操作示例
以 env_text/hi3519dv500/emmc_env.txt 为例：

### 3.1 生成 env.bin 和 ToolPlatform 分区表
```sh
make ENVTXT=env_text/hi3519dv500/emmc_env.txt all
```

### 3.2 仅生成 env.bin
- 生成 size 为 256KB（默认）的 env.bin
```sh
make ENVTXT=env_text/hi3519dv500/emmc_env.txt env
```
- 生成 size 为 1MB 的 env.bin
```sh
make ENVTXT=env_text/hi3519dv500/emmc_env.txt SIZE=0x100000 env
```

### 3.3 仅生成 ToolPlatform 分区表
```sh
make ENVTXT=env_text/hi3519dv500/emmc_env.txt burnxml
```

## 四、目录结构说明
```
uboot_env/
├── Makefile ----------- 编译脚本
├── REAME.md ----------- 使用说明
├── env_text ----------- 各产品 env 文本示例
└── scripts
    └── env2burn.py ---- 脚本用于生成 ToolPlatform 分区表
```

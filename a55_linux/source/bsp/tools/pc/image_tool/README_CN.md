## 一、概述

### 1.1 简介
本目录提供了一套制作启动镜像的脚本，下简称“脚本”。支持的镜像类别如下：
- 非安全（Non-Secure）启动镜像
- 安全（Secure）启动镜像
用户使用脚本制作镜像时，需要通过配置文件配入数据，配置文件的模板可由脚本生成。

### 1.2 范围
由于安全启动方案支持双签名，脚本分别为设备制造商（OEM）和设备使用者（Third party）提供了单签名镜像制作功能和双签名功能。

|     用户    |      涉及操作      |
|-------------|--------------------|
|     OEM     | 制作单签名镜像     |
| Third party | 对单签名镜像双签名 |

双签名是可选的，根据实际场景来使用。

### 1.3 安全启动特性
- 安全模式有两种：Non-Secure 和 Secure
- 启动流程（Start Flow ）有两种：Non-TEE 和 TEE ，启动顺序如下：

|  Start Flow  |                  启动顺序                |
|--------------|------------------------------------------|
|   Non-TEE    |  GSL -> U-Boot -> ATF -> Linux           |
|     TEE      |  GSL -> U-Boot -> ATF-> TEE-OS -> Linux  |

- 不同场景下脚本生成的镜像有如下特点：

|  Security Mode | Start Flow |           生成的镜像          | 签名 | 是否加密 | OP-TEE |
|----------------|------------|-------------------------------|------|----------|--------|
|   Non-Secure   |  Non-TEE   | boot_image.bin                |  无  |    否    |   无   |
|   Secure       |  Non-TEE   | boot_image.bin                |  有  |    是    |   无   |
|   Secure       |    TEE     | boot_image.bin、tee_image.bin |  有  |    是    |   有   |

说明：
(1) boot_image.bin包含GSL和U-Boot代码；
(2) tee_image.bin包含ATF和OP-TEE代码；
(3) Secure模式下，boot_image.bin中的GSL和U-Boot代码可配置不加密。

### 1.4 后文内容
- 第二节：镜像生成步骤，分别描述了OEM和Third party如何调用脚本生成配置文件和启动镜像。
- 第三节：镜像烧写说明；
- 第四节：目录结构说明，介绍了本目录下各子目录和文件的功能。


## 二、镜像生成步骤

### 2.1 OEM（生成启动镜像）

(1) 调用脚本生成配置文件模板（oem/oem_config.json）
```bash
$ python oem/oem_main.py gencfg oem/oem_config.json
```

(2) 根据《Hi35xxVxxx安全启动使用指南》中“镜像制作工具配置项”章节，填写替换oem/oem_config.json中由“/* */”括住的内容。

(3) 调用脚本生成启动镜像
```bash
$ python oem/oem_main.py build oem/oem_config.json
```

启动镜像的输出目录为“image/oem/”，如果需要Third party参与签名，需要将生成的启动镜像提供给Third party。

(4) otp 烧写及镜像检查(可选)

```bash
# non-tee
$ python oem/oem_main.py check oem/otp_check.json image/oem/boot_image.bin

# tee 使能, 且生成了 TEE 镜像.
$ python oem/oem_main.py check oem/otp_check.json image/oem/boot_image.bin image/oem/tee_image.bin
```

check 脚本对 boot 镜像和 teeos 镜像进行检查，并检查 otp 烧写值和镜像是否匹配。
其中:
- oem/otp_check.json 为 otp 配置文件，使用前需要根据 otp 的烧写值填写。
- image/oem/boot_image.bin 为启动镜像。
- image/oem/tee_image.bin(可选) 为 tee 镜像。

请注意 log 中的错误打印，check 工具遇到错误不会立刻停止，它会检查尽可能多的字段。

针对以下错误可以实现报错提醒:
- 校验 otp 使能, 情况下验签错误.
- 根(ree, tee)公钥区 hash 值不匹配.
- mrk1 otp 烧写错误, 镜像解密失败.
- tee 使能 otp 烧写但是未验签.
- dec_enable otp 烧写, 但是镜像里没做加密.

支持镜像全字段 dump，供客户调试自己的镜像制作脚本。
有定制化镜像制作脚本需求的用户可以自行删除以下位置的注释，以打印各个 area 中的字段。

```python
    def __check_flash_root_pub_key_area(self, flash_root_pub_key_area, image_bytes, hash_val):
        # flash_root_pub_key_area.dump()

    def __check_area(self, area, image_bytes):
        # area.dump()

    def __checkout_key_area(self, key_area, image_bytes, pub_key = None):
        # key_area.dump()

    def __checkout_code_info_area(self, info_area, image_bytes, pub_key = None):
        # info_area.dump()

    def __checkout_code_area(self, code_area, image_bytes, hash_val = None, dec_tool = None):
        # code_area.dump()
```

加解密场景下:
需要把 ../kdf_customer/ 目录中的 KDF_Linux_SecureBoot 和 parameter.bin 拷贝到本地目录(image_tool)。

### 2.2 Third party（双签名）

(1) 调用配置文件生成脚本，生成配置文件模板（third_party/third_party_config.json）
```bash
$ python third_party/third_party_main.py gencfg third_party/third_party_config.json
```

(2) 根据《Hi35xxVxxx安全启动使用指南》中“镜像制作工具配置项”章节，填写替换third_party/third_party_config.json中由“/* */”括住的内容。

(3) 将以下文件放入“image/oem/”目录
- boot_image.bin
- tee_image.bin （仅TEE场景需要）

(4) 调用脚本生成镜像
```bash
$ python third_party/third_party_main.py build third_party/third_party_config.json
```

双签名启动镜像的输出路径为“image/third_party/”；


## 三、镜像烧写说明

(1) 在 Non-TEE 启动场景下，需烧写如下内容：

|    烧写的镜像     | 包含的组件  |
|-------------------|-------------|
| boot_image.bin    | GSL、U-Boot |
| uImage_Hi3519DV500  | ATF、Linux  |
| RootFS镜像        | 根文件系统  |


(2) 在 TEE 启动场景下，需烧写如下镜像：

|     烧写的镜像    | 包含的组件  |
|-------------------|-------------|
| boot_image.bin    | GSL、U-Boot |
| uImage            |    Linux    |
| RootFS镜像        | 根文件系统  |
| tee_image.bin     | ATF、OP-TEE |


## 四、目录结构说明

顶层目录的子目录如下：
- common
- oem
- third_party
- image

### 4.1 common
提供生成镜像过程中用到的各种功能函数：
- util.py： 基础数据处理函数集合，提供2进制数据处理、大小端变换和文件操作等函数；
- check.py：与检查相关函数集合，提供数格式检查、文件检查等操作；
- area_tool.py：提供生成Area相关的功能，包括protection key加密、Root_Public_Key Area生成；
- config.py ： 提供配置文件相关的功能，例如配置文件的生成与解析、配置项错误检测等；
- logger.py：提供日志打印功能；
- secure.py：提供签名、哈希和加密等密码学算法功能；
- clean.py：提供清除脚本生成文件的功能。

### 4.2 oem
- oem_main.py
  OEM用户直接调用的脚本，用于生成OEM相关的Area和单签名镜像。
- oem_quick_build.py
  用于生成非安全启动镜像的脚本。
- quick_build_config.json
  用于生成非安全启动镜像的配置文件。
- tools目录
  oem_main.py调用的子脚本，提供生成配置文件模板、二进制Area文件和启动镜像的功能。
- tmp目录
  编译时生成，存放编译时生成的中间文件。

### 4.3 third_party
- third_party_main.py
  Third party用户直接调用的脚本，用于生成Third party相关的Area和双签名镜像。
- tools目录
  存放third_party_main.py调用的子脚本，提供生成配置文件模板、二进制Area文件和双签名启动镜像的功能。
- tmp目录
  编译时生成，存放编译时生成的中间文件。


### 4.4 image
存放生成的启动镜像。

# Storage

## 系统需求

Ubuntu 16.04 LTS 64bits

FUSE v29

## 编程说明

1. python 2.7和C混合编程

2. fuse部分编程通过`strstr`匹配路径地址，再计算比较`strlen`确定访问的具体位置（本文件夹还是子文件夹或文件）

## 编译说明

使用Makefile文件：

```sh
make
```

## 运行说明


挂载：将写好的文件系统挂载到该目录`fuse`上，务必加`-d`参数

```sh
./thfs fuse -d username password
```

卸载：在文件管理器里图形化弹出或者

```sh
sudo umount fuse
```

## 关键的参考资料

[FUSE编程](https://cloud.tencent.com/developer/article/1039270)

[FUSE编译](https://blog.csdn.net/zgl07/article/details/7558766)

[C调用Python](https://www.cnblogs.com/Hisin/archive/2012/02/27/2370590.html)

## 更多的参考资料

[使用 FUSE 开发自己的文件系统](https://www.ibm.com/developerworks/cn/linux/l-fuse/)

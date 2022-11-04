# http-server

## test

运行全部测试如下：

```bash
# 编译得到 http-server
make
# 使用 mininet 建立网络拓扑
sudo python ./topo.py

# 此时进入 mininet 的交互界面，以 mininet> 开头
# 在 h1 主机上，后台启动服务器
h1 ./http-server &
# 在 h2 主机上，运行测试脚本，如果通过则没有任何输出
h2 python ./test/test.py
# 在 h1 主机上，启动 Wireshark 进行抓包分析
h1 wireshark
# 在 h2 主机上，使用普通用户启动 vlc
h2 su <normal-user-name> -c vlc &

# 在 UI 界面上进行视频传输和抓包分析

# 退出 mininet 环境
exit
```

在 Wireshark 中，选择主机 h1 的网卡，设置 `port 80 or port 443` 进行过滤。  
在 vlc 中，在 `Media` -> `Open Network Stream` -> `Network` 中，
通过 `https://10.0.0.1/Mr_Grizz_Take_Me_Away.flv` 播放视频流。

## note

http 报文格式
<https://blog.csdn.net/u013914309/article/details/125036120>

https 连接
<https://blog.csdn.net/trw777/article/details/123692860>  
HTTPS 通信的 C++ 实现
<https://zhuanlan.zhihu.com/p/47925400>  
刨根问底系列之 https 详细握手过程
<https://zhuanlan.zhihu.com/p/157461830>

TCP “两次挥手”
<https://cloud.tencent.com/developer/article/2026712>
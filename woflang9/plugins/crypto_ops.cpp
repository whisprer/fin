// plugins/crypto_ops.cpp
#ifndef WOFLANG_PLUGIN_EXPORT
#  ifdef _WIN32
#    define WOFLANG_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#  else
#    define WOFLANG_PLUGIN_EXPORT extern "C"
#  endif
#endif

#include "core/woflang.hpp"
#include <array>
#include <cstdint>
#include <random>
#include <stack>
#include <stdexcept>
#include <string>
#include <vector>

namespace woflang {
static inline double need_num(const WofValue& v,const char* op){
    if(!v.is_numeric()) throw std::runtime_error(std::string(op)+": numeric required");
    return v.as_numeric();
}
static std::string to_hex_bytes(const std::vector<uint8_t>& b){ static const char* H="0123456789abcdef"; std::string o; o.reserve(b.size()*2); for(uint8_t x: b){ o.push_back(H[x>>4]); o.push_back(H[x&15]); } return o; }
static std::vector<uint8_t> from_hex_bytes(const std::string& s){ auto hv=[](char c)->int{ if(c>='0'&&c<='9') return c-'0'; if(c>='a'&&c<='f') return c-'a'+10; if(c>='A'&&c<='F') return c-'A'+10; return -1; }; if(s.size()%2) throw std::runtime_error("from_hex: odd length"); std::vector<uint8_t> o; o.reserve(s.size()/2); for(size_t i=0;i<s.size();i+=2){ int hi=hv(s[i]),lo=hv(s[i+1]); if(hi<0||lo<0) throw std::runtime_error("from_hex: bad digit"); o.push_back(uint8_t((hi<<4)|lo)); } return o; }
static const char* B64="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static std::string b64_encode(const std::vector<uint8_t>& d){ std::string o; o.reserve(((d.size()+2)/3)*4); size_t i=0; while(i+3<=d.size()){ uint32_t n=(d[i]<<16)|(d[i+1]<<8)|d[i+2]; i+=3; o.push_back(B64[(n>>18)&63]); o.push_back(B64[(n>>12)&63]); o.push_back(B64[(n>>6)&63]); o.push_back(B64[n&63]); } if(i+1==d.size()){ uint32_t n=(d[i]<<16); o.push_back(B64[(n>>18)&63]); o.push_back(B64[(n>>12)&63]); o.push_back('='); o.push_back('='); } else if(i+2==d.size()){ uint32_t n=(d[i]<<16)|(d[i+1]<<8); o.push_back(B64[(n>>18)&63]); o.push_back(B64[(n>>12)&63]); o.push_back(B64[(n>>6)&63]); o.push_back('='); } return o; }
static std::vector<uint8_t> b64_decode(const std::string& s){ std::array<int,256> T{}; T.fill(-1); for(int i=0;i<64;++i) T[uint8_t(B64[i])]=i; std::vector<uint8_t> o; int val=0,valb=-8; for(unsigned char c: s){ if(c=='=') break; int d=T[c]; if(d<0) continue; val=(val<<6)|d; valb+=6; if(valb>=0){ o.push_back(uint8_t((val>>valb)&0xFF)); valb-=8; } } return o; }
}

WOFLANG_PLUGIN_EXPORT void init_plugin(woflang::WoflangInterpreter::OpTable* ops){
    using namespace woflang;
    if (!ops) return;

    (*ops)["to_hex"] = [](std::stack<WofValue>& S){
        std::string s = S.top().to_string(); S.pop();
        std::vector<uint8_t> b(s.begin(), s.end()); S.push(WofValue(to_hex_bytes(b)));
    };
    (*ops)["from_hex"] = [](std::stack<WofValue>& S){
        std::string hx = S.top().to_string(); S.pop();
        auto bytes = from_hex_bytes(hx); S.push(WofValue(std::string(bytes.begin(), bytes.end())));
    };
    (*ops)["base64_encode"] = [](std::stack<WofValue>& S){
        std::string s = S.top().to_string(); S.pop();
        std::vector<uint8_t> b(s.begin(), s.end()); S.push(WofValue(b64_encode(b)));
    };
    (*ops)["base64_decode"] = [](std::stack<WofValue>& S){
        std::string s = S.top().to_string(); S.pop();
        auto bytes = b64_decode(s); S.push(WofValue(std::string(bytes.begin(), bytes.end())));
    };
    (*ops)["random"] = [](std::stack<WofValue>& S){
        static thread_local std::mt19937_64 rng{std::random_device{}()}; std::uniform_real_distribution<double> d(0.0,1.0);
        S.push(WofValue(d(rng)));
    };
    (*ops)["random_bytes"] = [](std::stack<WofValue>& S){
        int n = static_cast<int>(need_num(S.top(),"random_bytes")); S.pop();
        if(n<0 || n>1000000) throw std::runtime_error("random_bytes: n out of range");
        static thread_local std::mt19937_64 rng{std::random_device{}()}; std::uniform_int_distribution<int> d(0,255);
        std::string out; out.resize(n); for(int i=0;i<n;++i) out[i]=char(d(rng));
        S.push(WofValue(std::move(out)));
    };
}

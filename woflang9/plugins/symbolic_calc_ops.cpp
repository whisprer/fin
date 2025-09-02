// plugins/symbolic_calc_ops.cpp
#include <iostream>
#include <memory>
#include <string>
#include <stdexcept>

#ifndef WOFLANG_PLUGIN_EXPORT
#  ifdef _WIN32
#    define WOFLANG_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#  else
#    define WOFLANG_PLUGIN_EXPORT extern "C"
#  endif
#endif

WOFLANG_PLUGIN_EXPORT void init_plugin(woflang::WoflangInterpreter::OpTable* op_table) {

namespace woflang {

struct Expr {
    virtual std::string str() const=0;
    virtual std::unique_ptr<Expr> diff(const std::string& var) const=0;
    virtual ~Expr()=default;
};

struct Var:Expr{
    std::string name;
    Var(std::string n):name(std::move(n)){};
    std::string str()const override{ return name; }
    std::unique_ptr<Expr> diff(const std::string& var) const override{
        return std::make_unique<Var>(name==var?"1":"0");
    }
};

struct Const:Expr{
    double val;
    Const(double v):val(v){};
    std::string str()const override{ return std::to_string(val); }
    std::unique_ptr<Expr> diff(const std::string&) const override{
        return std::make_unique<Const>(0);
    }
};


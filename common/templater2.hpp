#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <sstream>


#define __TEMPLATER_R R

#define __TEMPLATER_BEGIN_QUOTE __TEMPLATER_R"(
#define __TEMPLATER_END_QUOTE )"

#define __TEMPLATER_CXX_INJECTION(...)  __TEMPLATER_END_QUOTE __VA_ARGS__ __TEMPLATER_BEGIN_QUOTE

#define $(...)      __TEMPLATER_CXX_INJECTION( << __VA_ARGS__ << )
#define $if(...)    __TEMPLATER_CXX_INJECTION( if (__VA_ARGS__) { res << )
#define $else       __TEMPLATER_CXX_INJECTION( } else { res << )
#define $end        __TEMPLATER_CXX_INJECTION( } res << )
#define $foreach(...)  __TEMPLATER_CXX_INJECTION( for (__VA_ARGS) { res << )


#define TEMPLATE_LAMBDA_BEGIN(CAPTURE)  [CAPTURE]() { std::stringstream res; res << __TEMPLATER_BEGIN_QUOTE 
#define TEMPLATE_LAMBDA_END()  __TEMPLATER_END_QUOTE; return res.str(); }


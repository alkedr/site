#pragma once

/*
 * DEFINE_STRUCT - macro that defines struct with minimal type introspection
 *
 * Example:
 *
 * DEFINE_STRUCT(
 *   MyCoolStruct,
 *   (int) a,
 *   (std::string) b,
 *   (std::map<int, double>) c
 * )
 *
 * will expand to:
 *
 * struct MyCoolStruct {
 *   int a;
 *   std::string b;
 *   std::map<int, double> c;
 *
 *   template<class F> void forEachField(F f) {
 *     f(this->a);
 *     f(this->b);
 *     f(this->c);
 *   }
 *
 *   bool operator==(const MyCoolStruct & other) {
 *     if (this->a != other.a) return false;
 *     if (this->b != other.b) return false;
 *     if (this->c != other.c) return false;
 *     return true;
 *   }
 * };
 */



#define BOOST_PP_VARIADICS
#include <boost/preprocessor.hpp>


#define __DEFINE_STRUCT_REMOVE_PARANTHESES(...) __VA_ARGS__
#define __DEFINE_STRUCT_REMOVE_TYPE(...)

#define __DEFINE_STRUCT_MEMBER(R, DATA, ITEM) __DEFINE_STRUCT_REMOVE_PARANTHESES ITEM;

#define __DEFINE_STRUCT_FOREACH(R, DATA, ITEM) f(this->__DEFINE_STRUCT_REMOVE_TYPE ITEM);

#define __DEFINE_STRUCT_OPERATOR_EQUALS(R, DATA, ITEM) if (this->__DEFINE_STRUCT_REMOVE_TYPE ITEM != other. __DEFINE_STRUCT_REMOVE_TYPE ITEM) return false;

#define DEFINE_STRUCT(NAME, ...)                                                                                                 \
struct NAME {                                                                                                                    \
BOOST_PP_SEQ_FOR_EACH(__DEFINE_STRUCT_MEMBER, ~, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))                                          \
template<class F> void forEachField(F f) {BOOST_PP_SEQ_FOR_EACH(__DEFINE_STRUCT_FOREACH, ~, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))}   \
template<class F> void forEachField(F f) const {BOOST_PP_SEQ_FOR_EACH(__DEFINE_STRUCT_FOREACH, ~, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))}   \
bool operator==(const NAME & other) const {BOOST_PP_SEQ_FOR_EACH(__DEFINE_STRUCT_OPERATOR_EQUALS, ~, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) return true;}   \
};


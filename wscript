#!/usr/bin/env python

VERSION='0.0.1'
APPNAME='site'

out = 'build'

def options(opt):
	opt.load('compiler_cxx waf_unit_test')

def configure(conf):
	conf.load('compiler_cxx waf_unit_test')
	conf.env.CXXFLAGS = ['-std=c++11', '-Weverything', '-Wno-padded', '-Wno-c++98-compat', '-Wno-c++98-compat-pedantic', '-Wno-c99-extensions', '-Wno-exit-time-destructors', '-Wno-missing-variable-declarations', '-Wno-weak-vtables', '-Wno-sign-conversion', '-Wno-float-equal']
	conf.env.INCLUDES = ['..']

def build(bld):
	bld.recurse(['common', 'lfdb', 'htmld', 'filmd'])


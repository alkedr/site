#!/usr/bin/env python

VERSION='0.0.1'
APPNAME='site'

out = 'build'

def options(opt):
	opt.load('compiler_cxx waf_unit_test')

def configure(conf):
	conf.load('compiler_cxx waf_unit_test')
	conf.env.CXXFLAGS = ['-std=c++11 -Weverything']
	conf.env.INCLUDES = ['..']

def build(bld):
	bld.recurse(['common', 'lfdb', 'htmld', 'filmd'])


#! /usr/bin/env python

from distutils.core import setup, Extension

setup (name = "matplc",
       version = "0.1",
       author = "Jiri Baum",
       author_email = "jiri@baum.com.au",
       description = "Python interface for MatPLC",
       url = "http://mat.sf.net",

       py_modules = ['matplc'],
       ext_modules = [Extension('matplc_internal',
                                sources = ['matplcmodule.c'],
				include_dirs = ['../../lib'],
				libraries = ['matplc', 'rt'],
				library_dirs = ['../../lib/.libs'],
			       )]
)

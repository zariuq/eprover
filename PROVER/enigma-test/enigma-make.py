#!/usr/bin/python

import os
import sys
from atpy.enigma import models

f_pre = sys.argv[1]
name = "test"

os.system("rm -fr %s" % models.path(name))
os.system("mkdir -p %s" % models.path(name))
os.system("cp %s %s" % (f_pre, models.path(name, "train.pre")))
models.standard(name)


#!/usr/bin/python

import sys
import os

MODEL="""solver_type L2R_L2LOSS_SVC
nr_class 2
label 1 10
nr_feature %s
bias -1
w
%s"""

def load_map(f_map):
   nums = {}
   for line in file(f_map):
      (fid,ftr) = line.strip().split("(")[1].split(",")
      fid = int(fid.strip(", "))
      ftr = ftr.strip('") .')
      nums[ftr] = fid
   return nums

def dump_map(out, nums):
   rev = {nums[x]:x for x in nums}
   for x in sorted(rev.keys()):
      out.write('feature(%s, "%s").\n' % (x,rev[x]))

def dump_model(out, model):
   nums = "\n".join(["%.12f"%model[x] if model[x]!=0 else "0.0" for x in sorted(model)])
   out.write(MODEL % (len(model),nums))
   #for n in sorted(model):
   #   out.write("%d: %s\n" % (n,model[n]))

def reverse(m):
   return {m[x]:x for x in m}

def load_maps(names):
   gmap = {}
   maps = {}
   for name in names:
      f_map = os.path.join(name, "enigma.map")
      enimap = load_map(f_map)
      maps[name] = reverse(enimap)
      for ftr in enimap:
         if ftr not in gmap:
            gmap[ftr] = len(gmap)+1
   return (gmap, maps)

def update_joint(name, gmap, maps, joint):
   f = file(os.path.join(name, "model.lin"))
   for line in f:
      line = line.strip()
      if line.startswith("label"):
         if line != "label 1 10":
            print "Warning: Wrong labels '%s' in model %s" % (line, name)
      if line == "w":
         break
   
   i = 1
   for line in f:
      w = float(line.strip())
      ftr = maps[name][i]
      if w != 0:
         joint[gmap[ftr]].append(w)
      i += 1
      if i > len(maps[name]):
         break

   i = 1
   for line in f:
      w = float(line.strip())
      ftr = maps[name][i]
      if w != 0:
         joint[gmap[ftr]+len(gmap)].append(w)
      i += 1

   f.close()

def joint_model(names, gmap, maps):
   joint = {n:[] for n in xrange(1,2*len(gmap)+1)}
   for name in names:
      update_joint(name, gmap, maps, joint)
      #print "enigma-join:   * joint model with %s (%d non-empty features)" % (name, len([x for x in joint if joint[x]]))
   return joint

def dump_joint(out, joint, gmap, names):
   #os.system("rm -fr %s" % out)
   os.system("mkdir -p %s" % out)
   dump_map(file(os.path.join(out,"enigma.map"),"w"), gmap)
   dump_model(file(os.path.join(out,"model.lin"),"w"), joint)
   file(os.path.join(out,"names.txt"),"w").write("\n".join(names))

def avg(xs):
   return float(sum(xs))/len(xs)

combine = None
if len(sys.argv) > 2:
   print "enigma-join: combine function %s" % sys.argv[1]
   if sys.argv[1] == "avg":
      combine = avg
   elif sys.argv[1] == "max":
      combine = max

if len(sys.argv) < 4 or not combine:
   print "usage: enigma-join [avg|max] out_dir model_dir1 model_dir2 ..."
else:
   out = sys.argv[2]
   names = sys.argv[3:]
   print "enigma-join: loading %d models" % len(names)
   (gmap, maps) = load_maps(names)
   joint = joint_model(names, gmap, maps)
   joint = {x:combine(joint[x]) if joint[x] else 0.0 for x in joint}
   dump_joint(out, joint, gmap, names)
   print "enigma-join: model dumped to %s" % out


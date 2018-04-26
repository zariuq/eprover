#!/usr/bin/python

import sys
import os

OUT_TRAIN="train.in"
OUT_MAP="enigma.map"

PREFIX = {
   "+": "+1",
   "-": "+10",
   "*": "Should not ever see me"
}

def get_features(f):
   features = set()
   for line in file(f):
      (sign,clause,conj) = line.strip().split("|")
      for feature in clause.strip().split(" "):
         features.add(feature)
      for feature in conj.strip().split(" "):
         features.add(feature)
   return features

def number_features(features):
   return {y:x for (x,y) in enumerate(features, start=1)}

def load_map(f_map):
   nums = {}
   if not os.path.exists(f_map):
      return nums
   for line in file(f_map):
      (fid,ftr) = line.strip().split("(")[1].split(",")
      fid = int(fid.strip(", "))
      ftr = ftr.strip('") .')
      nums[ftr] = fid
   return nums

def get_map(f_train, f_map):
   if f_map:
      return load_map(f_map)
   else:
      #return load_map(OUT_MAP)
      features = get_features(f_train)
      return number_features(features)

def encode_features(ftrs, nums, offset):
   counts = {}
   for ftr in ftrs:
      if ftr not in nums:
         nums[ftr] = len(nums)+1
      fid = nums[ftr]
      counts[fid] = counts[fid]+1 if fid in counts else 1
   tmp = sorted([(fid,counts[fid]) for fid in counts])
   tmp = ["%s:%s"%(fid+offset,cnt) for (fid,cnt) in tmp]
   return " ".join(tmp)

def dump_train(out, f, nums, conj_offset):
   for line in file(f):
      (sign,clause,conj) = line.strip().split("|")
      clause_ftrs = encode_features(clause.strip().split(" "), nums, 0)
      conj_ftrs = encode_features(conj.strip().split(" "), nums, conj_offset) if conj else ""
      out.write("%s %s %s\n" % (PREFIX[sign], clause_ftrs, conj_ftrs))

def dump_map(out, nums):
   rev = {nums[x]:x for x in nums}
   for x in sorted(rev.keys()):
      out.write('feature(%s, "%s").\n' % (x,rev[x]))



if len(sys.argv) != 2 and len(sys.argv) != 3:
   print "usage: %s train.pre [enigma.map]" % sys.argv[0]
else:
   me = os.path.basename(sys.argv[0])
   f_train = sys.argv[1]
   f_map = sys.argv[2] if len(sys.argv) == 3 else None

   nums = get_map(f_train, f_map)
   print "%s: input features count: %d" % (me,len(nums))
   print "%s: features input: %s" % (me, f_train)
   
   if f_map:
      print "%s: feature map update mode: %s" % (me, f_map)
      dump_train(file(os.devnull,"w"), f_train, nums, len(nums))
      print "%s: output features count: %d" % (me,len(nums))
   else:
      print "%s: feature traslate mode" % (me)
      dump_train(file(OUT_TRAIN,"w"), f_train, nums, len(nums))
      print "%s: training data for liblinear dumped to: %s" % (me,OUT_TRAIN)
      f_map = OUT_MAP

   dump_map(file(f_map,"w"), nums)
   print "%s: features map dumped to: %s" % (me,f_map)


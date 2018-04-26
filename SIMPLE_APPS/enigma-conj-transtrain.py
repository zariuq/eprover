#!/usr/bin/python

import sys

OUT_TRAIN="train.in"

POS="+1"
NEG="+10"

def encode_features(ftrs, nums, offset):
   counts = {}
   for ftr in ftrs:
      fid = nums[ftr]
      counts[fid] = counts[fid]+1 if fid in counts else 1
   tmp = sorted([(fid,counts[fid]) for fid in counts])
   tmp = ["%s:%s"%(fid+offset,cnt) for (fid,cnt) in tmp]
   return " ".join(tmp)

def dump_train(out, f, nums, conj_offset):
   for line in file(f):
      (sign,clause,conj) = line.strip().split("|")
      clause_ftrs = encode_features(clause.strip().split(" "), nums, 0)
      conj_ftrs = encode_features(conj.strip().split(" "), nums, conj_offset)
      if sign=="+":
         out.write("%s %s %s\n" % (POS, clause_ftrs, conj_ftrs))
      else:
         out.write("%s %s %s\n" % (NEG, clause_ftrs, conj_ftrs))

def read_map(f):
   nums = {}
   for line in file(f):
      line = line.strip()
      line = line.split("(")[1]
      line = line.rstrip(").")
      (number,feature) = line.split(",")
      feature = feature.strip(' "')
      nums[feature] = int(number)
   return nums

if len(sys.argv) != 3:
   print "usage: %s train.pre enigma.map" % sys.argv[0]
else:
   me = sys.argv[0]
   inp = sys.argv[1]
   inm = sys.argv[2]

   print "%s: translating input %s with map %s" % (me, inp, inm)
   nums = read_map(inm)
   print "%s: features count: %d" % (me,len(nums))
   dump_train(file(OUT_TRAIN,"w"), inp, nums, len(nums))
   print "%s: training data for liblinear dumped to: %s" % (me,OUT_TRAIN)


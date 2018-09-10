#!/usr/bin/python

import sys

OUT_TRAIN="train.in"
OUT_MAP="enigma.map"

POS="+1"
NEG="+10"

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

def encode_features(ftrs, nums, offset):
   counts = {}
   for ftr in ftrs:
      fid = nums[ftr]
      counts[fid] = counts[fid]+1 if fid in counts else 1
   tmp = sorted([(fid,counts[fid]) for fid in counts])
   tmp = ["%s:%s"%(fid+offset,cnt) for (fid,cnt) in tmp]
   return " ".join(tmp)

def dump_train(out, f, nums, conj_offset, boost):
   for line in file(f):
      (sign,clause,conj) = line.strip().split("|")
      clause_ftrs = encode_features(clause.strip().split(" "), nums, 0)
      conj_ftrs = encode_features(conj.strip().split(" "), nums, conj_offset)
      if sign=="+":
         for i in range(boost):
            out.write("%s %s %s\n" % (POS, clause_ftrs, conj_ftrs))
      else:
         out.write("%s %s %s\n" % (NEG, clause_ftrs, conj_ftrs))

def dump_map(out, nums):
   rev = {nums[x]:x for x in nums}
   for x in sorted(rev.keys()):
      out.write('feature(%s, "%s").\n' % (x,rev[x]))

if len(sys.argv) < 2:
   print "usage: %s enigma-conj-features.out [boost10]" % sys.argv[0]
else:
   me = sys.argv[0]
   inp = sys.argv[1]
   boost = 1
   if len(sys.argv)==3 and sys.argv[2]=="boost10":
      boost = 10

   print "%s: processing input: %s" % (me, inp)
   features = get_features(inp)
   print "%s: features count: %d" % (me,len(features))
   nums = number_features(features)
   dump_train(file(OUT_TRAIN,"w"), inp, nums, len(features), boost)
   print "%s: training data for liblinear dumped to: %s" % (me,OUT_TRAIN)
   dump_map(file(OUT_MAP,"w"), nums)
   print "%s: features map dumped to: %s" % (me,OUT_MAP)


#!/usr/bin/python

import sys
import os

OUT_TRAIN="train.in"
OUT_MAP="enigma.map"
OUT_SVD="svd.in"

POS="+1"
NEG="+10"

NONZEROS=0
DOCCOUNT=0

def add_occurence(feature, doc, occurs):
   global NONZEROS
   if doc not in occurs:
      occurs[doc] = {}
   if feature in occurs[doc]:
      occurs[doc][feature] += 1
   else:
      occurs[doc][feature] = 1
      NONZEROS += 1

def get_features(f):
   global DOCCOUNT 
   features = set()
   occurs_clause = {}
   occurs_conj = {}
   doc = 0
   for line in file(f):
      doc += 1
      (sign,clause,conj) = line.strip().split("|")
      for feature in clause.strip().split(" "):
         features.add(feature)
         add_occurence(feature, doc, occurs_clause)
      for feature in conj.strip().split(" "):
         features.add(feature)
         add_occurence(feature, doc, occurs_conj)
   DOCCOUNT = doc
   return (occurs_clause, occurs_conj, features)

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

def dump_train(out, f, nums, conj_offset):
   for line in file(f):
      (sign,clause,conj) = line.strip().split("|")
      clause_ftrs = encode_features(clause.strip().split(" "), nums, 0)
      conj_ftrs = encode_features(conj.strip().split(" "), nums, conj_offset)
      out.write("%s %s %s\n" % (POS if sign=="+" else NEG, clause_ftrs, conj_ftrs))

def dump_map(out, nums):
   rev = {nums[x]:x for x in nums}
   for x in sorted(rev.keys()):
      out.write('feature(%s, "%s").\n' % (x,rev[x]))

def dump_svd_sparse(out, nums, occurs_clause, occurs_conj):
   global DOCCOUNT, NONZEROS

   offset = len(nums)
   fmap = {nums[f]:f for f in nums}
   out.write("%d %d %d\n" % (2*len(nums), DOCCOUNT, NONZEROS))
   for did in xrange(1,DOCCOUNT+1):
      #print did, occurs_clause[did], occurs_conj[did]
      
      clause_features = [nums[fid] for fid in occurs_clause[did]]
      conj_features = [nums[fid] for fid in occurs_conj[did]]

      out.write("%s\n" % (len(clause_features)+len(conj_features)))
      for fid in sorted(clause_features):
         out.write("%s %s\n" % (fid-1, occurs_clause[did][fmap[fid]]))
      for fid in sorted(conj_features):
         out.write("%s %s\n" % (fid+offset-1, occurs_conj[did][fmap[fid]]))

      #docs = occurs_clause[fmap[i]]
      #print [(j,docs[j]) for j in sorted(docs)]

if len(sys.argv) != 2:
   print "usage: %s enigma-conj-features.out" % sys.argv[0]
else:
   me = os.path.basename(sys.argv[0])
   inp = sys.argv[1]
   print "%s: processing input: %s" % (me, inp)
   (occurs_clause, occurs_conj, features) = get_features(inp)
   print "%s: features count: %d" % (me,len(features))
   print "%s: documents count: %d" % (me,DOCCOUNT)
   print "%s: sparse factor: %d/%d" % (me,NONZEROS,2*len(features)*DOCCOUNT)
   nums = number_features(features)

   dump_svd_sparse(file(OUT_SVD,"w"), nums, occurs_clause, occurs_conj)
   print "%s: input for libsvd dumped to: %s" % (me,OUT_SVD)
   dump_train(file(OUT_TRAIN,"w"), inp, nums, len(features))
   print "%s: training data for liblinear dumped to: %s" % (me,OUT_TRAIN)
   dump_map(file(OUT_MAP,"w"), nums)
   print "%s: features map dumped to: %s" % (me,OUT_MAP)


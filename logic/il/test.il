### test program ###
# 4.1.00
# modified 1.7.00

LD X001
AND Y002
LD X002
ANI Y001
ORB
OUT Y004
SET R5
RST Y002
JSR test_sub

SUB test_sub
LD X001
RST R5

###

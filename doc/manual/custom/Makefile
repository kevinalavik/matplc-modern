# note: this must be re-made manually when chaser.c changes
all: example.html

example.html: ../../../demo/basic/chaser.c example.top example.bottom
	c2html -s -h example.top -b example.bottom < $< > $@

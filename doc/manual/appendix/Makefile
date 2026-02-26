# note: the "all" target is made on SF after each webpage update (8-hourly)
all: matplc.conf.html

%.html: % txt2html.pl
	./txt2html.pl $< > $@


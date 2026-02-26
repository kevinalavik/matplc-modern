default: manual.ps.gz manual.pdf

clean:
	-rm -f manual.ps manual.ps.gz manual.pdf
	-rm -f *.depend manual.log

manual.ps manual.depend: index.html
	html2ps -W bL -D -R -H -n -i 0.5 -o manual.ps $< 2>&1 | tee manual.log
	grep ^Reading manual.log | sed s/Reading/$@:/ > manual.depend

%.gz: %
	gzip < $< > $@

%.pdf: %.ps
	ps2pdf $< $@

-include *.depend

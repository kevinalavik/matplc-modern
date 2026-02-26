default: talkslides.ps

%.eps: %.jpg
	jpeg2ps $< > $@

%.jpg: %.gif
	giftopnm $< | cjpeg -quality 100 > $@

%.jpg: %.png
	pngtopnm $< | cjpeg -quality 100 > $@

%.dvi: %.ltx
	latex $<

%.ps: %.dvi
	dvips -o $@ $<

enlarged-logo.jpg MAT-linux-h100.gif:
	ln -s ../../images/$@ 

lplc-article-fig1.jpg lplc-article-fig2.jpg:
	ln -s ../../$@ 

talkslides.dvi: enlarged-logo.eps MAT-linux-h100.eps
talkslides.dvi: b-prd-index-plc.eps difulad.eps scan4.eps
talkslides.dvi: labview.eps grafcet.eps
talkslides.dvi: classicladder_snapshot1.eps heater_ss.eps oven_ss.eps
talkslides.dvi: lplc-article-fig1.eps lplc-article-fig2.eps
talkslides.dvi: slideformat.sty mydefs.tex

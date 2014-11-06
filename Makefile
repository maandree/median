# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.  This file is offered as-is,
# without any warranty.



.PHONY: default
default: info

.PHONY: all
all: doc

.PHONY: doc
doc: info pdf dvi ps

.PHONY: info
info: median.info
%.info: info/%.texinfo info/fdl.texinfo
	makeinfo $<

.PHONY: pdf
pdf: median.pdf
%.pdf: info/%.texinfo info/fdl.texinfo
	@mkdir -p obj
	cd obj ; yes X | texi2pdf ../$<
	mv obj/$@ $@

.PHONY: dvi
dvi: median.dvi
%.dvi: info/%.texinfo info/fdl.texinfo
	@mkdir -p obj
	cd obj ; yes X | $(TEXI2DVI) ../$<
	mv obj/$@ $@

.PHONY: ps
ps: median.ps
%.ps: info/%.texinfo info/fdl.texinfo
	@mkdir -p obj
	cd obj ; yes X | texi2pdf --ps ../$<
	mv obj/$@ $@


.PHONY: clean
clean:
	-rm -r obj bin *.info *.pdf *.dvi *.ps


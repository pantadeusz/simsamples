
DOCNAME=$(shell ls | grep -E '.tex$$' | sed 's/[.]tex$$//' | head -n 1)

all: dependencies $(DOCNAME).pdf

$(DOCNAME).pdf: $(DOCNAME).tex
	latexmk -pdf $(DOCNAME).tex

distclean: clean dependencies-clean
	rm -f $(DOCNAME).pdf

clean:
	rm -f $(DOCNAME).pdf $(DOCNAME).idx  $(DOCNAME).ilg  $(DOCNAME).ind  $(DOCNAME).aux  $(DOCNAME).bbl  $(DOCNAME).blg  $(DOCNAME).fdb_latexmk $(DOCNAME).fls  $(DOCNAME).log $(DOCNAME).nav  $(DOCNAME).snm  $(DOCNAME).toc $(DOCNAME).out  $(DOCNAME).vrb

###### ZALEZNOSCI - sciagane automatycznie
dependencies: pdfdepfind

pdfdepfind:
	wget -N https://raw.githubusercontent.com/pantadeusz/teptex/master/scripts/pdfdepfind; \
	chmod a+x pdfdepfind
	./pdfdepfind $(DOCNAME).tex

dependencies-clean:
	./pdfdepfind $(DOCNAME).tex clean
	rm -f pdfdepfind

updatedep: dependencies
	./pdfdepfind $(DOCNAME).tex update

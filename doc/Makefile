# Makefile for Sphinx documentation
#

# You can set these variables from the command line.
SPHINXOPTS    =
SPHINXBUILD   = sphinx-build
PAPER         =
BUILD_DIR	  = build

# Internal variables.
PAPEROPT_a4     = -D latex_paper_size=a4
PAPEROPT_letter = -D latex_paper_size=letter
ALLSPHINXOPTS   = -d $(BUILD_DIR)/doctrees $(PAPEROPT_$(PAPER)) $(SPHINXOPTS) src

.PHONY: help clean html web htmlhelp latex changes linkcheck

help:
	@echo "Please use \`make <target>' where <target> is one of"
	@echo "  html      to make standalone HTML files"
	@echo "  web       to make files usable by Sphinx.web"
	@echo "  htmlhelp  to make HTML files and a HTML help project"
	@echo "  latex     to make LaTeX files, you can set PAPER=a4 or PAPER=letter"
	@echo "  changes   to make an overview over all changed/added/deprecated items"
	@echo "  linkcheck to check all external links for integrity"

clean:
	-rm -rf $(BUILD_DIR)

html:
	mkdir -p $(BUILD_DIR)/doctrees
	$(SPHINXBUILD) -b html $(ALLSPHINXOPTS) $(BUILD_DIR)/html
	@echo
	@echo "Build finished. The HTML pages are in $(BUILD_DIR)/html"

web:
	mkdir -p $(BUILD_DIR)/web $(BUILD_DIR)/doctrees
	$(SPHINXBUILD) -b web $(ALLSPHINXOPTS) $(BUILD_DIR)/web
	@echo
	@echo "Build finished; now you can run"
	@echo "  python -m sphinx.web $(BUILD_DIR)/web"
	@echo "to start the server."

htmlhelp:
	mkdir -p $(BUILD_DIR)/htmlhelp $(BUILD_DIR)/doctrees
	$(SPHINXBUILD) -b htmlhelp $(ALLSPHINXOPTS) $(BUILD_DIR)/htmlhelp
	@echo
	@echo "Build finished; now you can run HTML Help Workshop with the" \
	      ".hhp project file in $(BUILD_DIR)/htmlhelp."

latex:
	mkdir -p $(BUILD_DIR)/latex $(BUILD_DIR)/doctrees
	$(SPHINXBUILD) -b latex $(ALLSPHINXOPTS) $(BUILD_DIR)/latex
	@echo
	@echo "Build finished; the LaTeX files are in $(BUILD_DIR)/latex."
	@echo "Run \`make all-pdf' or \`make all-ps' in that directory to" \
	      "run these through (pdf)latex."

changes:
	mkdir -p $(BUILD_DIR)/changes $(BUILD_DIR)/doctrees
	$(SPHINXBUILD) -b changes $(ALLSPHINXOPTS) $(BUILD_DIR)/changes
	@echo
	@echo "The overview file is in $(BUILD_DIR)/changes."

linkcheck:
	mkdir -p $(BUILD_DIR)/linkcheck $(BUILD_DIR)/doctrees
	$(SPHINXBUILD) -b linkcheck $(ALLSPHINXOPTS) $(BUILD_DIR)/linkcheck
	@echo
	@echo "Link check complete; look for any errors in the above output " \
	      "or in $(BUILD_DIR)/linkcheck/output.txt."

# Minimal makefile for Sphinx documentation
#

# You can set these variables from the command line.
SPHINXOPTS       =
SPHINXBUILD      = python -msphinx
SPHINXPROJ       = reactions
SOURCEDIR        = source
BUILDDIR        := $(if $(BUILDDIR),$(BUILDDIR),../../reactions-docs)
GITCOMMANDS      = cd $(BUILDDIR)/html; git add . ; git commit -m 'Rebuilt docs' ; git push origin gh-pages
PROJECT_SOURCES  = $(shell find ../python/reactions -type f)

# Put it first so that "make" without argument is like "make help".
help:
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)
	@echo 'If the target is "gh-pages", "html" will be built, and the '\
	'result will be automatically published through the command'
	@echo "$(GITCOMMANDS)"

.PHONY: help Makefile

# Target to build the GitHub Pages
gh-pages: html
	$(GITCOMMANDS)

# Forces running certain rules
FORCE: ;

# Catch-all target: route all unknown targets to Sphinx using the new
# "make mode" option.  $(O) is meant as a shortcut for $(SPHINXOPTS).
%-doc: Makefile $(PROJECT_SOURCES)
	@if [ -d $(BUILDDIR)/doctrees ]; then rm -r $(BUILDDIR)/doctrees; fi
	@if [ -d source/api ]; then rm -r source/api; fi
	@$(SPHINXBUILD) -M $(subst -doc,,$@) "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

# Shortcuts
html: html-doc;

latexpdf: latexpdf-doc;

clean:
	@rm -rf source/_static/cpp source/api

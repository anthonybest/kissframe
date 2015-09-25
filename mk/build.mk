# Build directory Maike file
# Anthony Best <abest@digitalflex.net>
# Almost entirely based on Paul D. Smith <psmith@gnu.org>
# http://make.paulandlesley.org/multi-arch.html
# slightly modified.
#
# Alows make to run in different directory then sources/root build
.SUFFICES:

OBJDIR := $(BUILDPATH)

MAKETARGET = $(MAKE) --no-print-directory -C $@ -f $(BUILDMAKEFILE) \
			SRCDIR=$(CURDIR)/src \
			BUILDMAKEFILE=$(BUILDMAKEFILE) \
			FOOFLAGS=one \
			OTHER_MAKEFILES="$(abspath $(MAKEFILE_LIST))" $(MAKECMDGOALS)

.PHONY: $(OBJDIR)

$(OBJDIR):
	+@[ -d $@ ] || mkdir -p $@
	+@$(MAKETARGET)

Makefile : ;

#%mk :: ;

$ :: $(OBJDIR) ; :

.PHONY: clean

clean:
	rm -rf $(OBJDIR)


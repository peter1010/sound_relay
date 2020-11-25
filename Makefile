#
# Thanks goes to http://make.paulandlesley.org for the ideas and concepts that
# I've borrowed in these makefiles
#
# Check if already in the target build directory
# Target build directories start __

BASE_CURDIR=$(notdir $(CURDIR))
ifeq (,$(filter __%, $(BASE_CURDIR)))
    include target.mk
else
    VPATH = $(SRCDIR)
    include $(SRCDIR)/build.mk
endif

#####################################################################################################################
##                                                                                                                 ##
## makefile -- Used to control the build for the "ast-cc" compiler.                                                ##
##                                                                                                                 ##
##    Date     Tracker  Version  Pgmr  Modification                                                                ##
## ----------  -------  -------  ----  --------------------------------------------------------------------------- ##
## 2016-02-07  Initial   v0.0    ADCL  Initial version -- leveraged from the pascal compiler, but made C++         ##
## 2016-03-29    N/A     v0.1    ADCL  This is a rewrite of the ast-cc compiler.  I am updating the source files   ##
##                                     to be more in line with the naming conventions for the pascal compiler.     ##
##                                     As a result this file changes with the rules (and by the way, adding        ##
##                                     cppcheck into the mix)                                                      ##
##                                                                                                                 ##
#####################################################################################################################

.SILENT:


VER=0.1


#
# -- Some strings needed to complete the dependency analysis
#    -------------------------------------------------------
YY-SRC=$(sort $(wildcard *.yy))
YY-CC=$(subst .yy,.cc,$(YY-SRC))
YY-HH=$(subst .yy,.hh,$(YY-SRC))
YY-D=$(subst .yy,.d,$(YY-SRC))

HH-SRC=$(sort $(wildcard *.h) $(YY-HH))

LL-SRC=$(sort $(wildcard *.ll))
LL-CC=$(subst .ll,.cc,$(LL-SRC))
LL-D=$(subst .ll,.d,$(LL-SRC))

CC-SRC=$(sort $(wildcard *.cc) $(YY-CC) $(LL-CC))

ALL-SRC=$(sort $(YY-SRC) $(LL-SRC) $(wildcard *.cc) $(wildcard *.h))

OBJ=$(subst .cc,.o,$(CC-SRC))
DEP=$(subst .o,.d,$(OBJ))

RPT=ast-cc.rpt

TGT=ast-cc

#
# -- These are the build commands
#    ----------------------------
CFLAGS=-Wno-write-strings -Wno-unused-function
LIBS=-lstdc++ -lm

LEX=flex --yylineno
YAC=bison --defines=$(YY-HH) --debug --report-file=$(RPT) --report=all
GCC=gcc -c -Wall -g -Iinclude $(CFLAGS)
LD=gcc

#
# -- Finally, these are the recipes needed to build the actual system
#    ----------------------------------------------------------------
all: $(TGT) makefile
#	echo "Headers " $(HH-SRC)
#	echo "Scanner " $(LL-SRC)
#	echo "Grammar " $(YY-SRC)
#	echo "C++ Src " $(CC-SRC)
#	echo "Objects " $(OBJ)
#	echo "All Src " $(ALL-SRC)

ifneq ($(MAKECMDGOALS),clean)
-include $(DEP)
endif

$(TGT): $(OBJ) makefile
	echo "LINK  " $@
	$(LD) -o $@ $(OBJ) $(LIBS)

%.o: %.cc
	echo "CC    " $<
	$(GCC) -o $@ $<

%.cc: %.yy
	echo "YACC  " $<
	$(YAC) -o $@ $<

%.cc: %.ll
	echo "LEX   " $<
	$(LEX) -o $@ $<

%.d: %.cc
	echo "DEPEND" $(notdir $<)
	$(GCC) -MM -MG $< | sed -e 's@^\(.*\)\.o:@\1.d \1.o:@' > $@

$(YY-D) $(YY-HH) $(YY-CC): $(YY-SRC)
$(LL-D) $(LL-CC): $(LL-SRC)

clean:
	rm -f $(TGT)
	rm -f $(YY-CC)
	rm -f $(YY-HH)
	rm -f $(LL-CC)
	rm -f $(OBJ)
	rm -f $(RPT)
	rm -f $(DEP)
	rm -f *~
	rm -f .gitignore~
	rm -f ast-nodes.hh
	echo "All cleaned up!"

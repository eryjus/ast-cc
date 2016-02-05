#####################################################################################################################
##                                                                                                                 ##
## makefile -- Used to control the build for the "pascal-cc" compiler.                                             ##
##                                                                                                                 ##
##    Date     Tracker  Version  Pgmr  Modification                                                                ##
## ----/--/--  -------  -------  ----  --------------------------------------------------------------------------- ##
## 2016/02/07  Initial   v0.0    ADCL  Initial version -- leveraged from the pascal compiler, but made C++         ##
##                                                                                                                 ##
#####################################################################################################################

.SILENT:


VER=0.0


#
# -- Some strings needed to complete the dependency analysis
#    -------------------------------------------------------
YY-SRC=$(sort $(wildcard *.y))
YY-CC=$(subst .y,.cc,$(YY-SRC))
YY-HH=$(subst .y,.hh,$(YY-SRC))

HH-SRC=$(sort $(wildcard *.h) $(YY-HH))

LL-SRC=$(sort $(wildcard *.l))
LL-CC=$(subst .l,.cc,$(LL-SRC))

CC-SRC=$(sort $(wildcard *.cc) $(YY-CC) $(LL-CC))

ALL-SRC=$(sort $(YY-SRC) $(LL-SRC) $(wildcard *.cc) $(wildcard *.h))

OBJ=$(subst .cc,.o,$(CC-SRC))

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

$(TGT): $(OBJ) makefile
	echo "LINK  " $@
	$(LD) -o $@ $(OBJ) $(LIBS)

%.o: %.cc
	echo "CC    " $<
	$(GCC) -o $@ $<

%.cc: %.y
	echo "YACC  " $<
	$(YAC) -o $@ $<

%.cc: %.l
	echo "LEX   " $<
	$(LEX) -o $@ $<

$(YY-HH) $(YY-CC): $(YY-SRC)
$(LL-CC): $(LL-SRC)

clean:
	rm -f $(TGT)
	rm -f $(YY-CC)
	rm -f $(YY-HH)
	rm -f $(LL-CC)
	rm -f $(OBJ)
	rm -f $(RPT)
	rm -f *~
	rm -f .gitignore~
	echo "All cleaned up!"

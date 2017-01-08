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
## 2017-01-07    #314   v0.1.1   ADCL  Rework the project folder layout; fix issue with .h/.hh files               ## 
##                                                                                                                 ##
#####################################################################################################################

.SILENT:


VER=0.1.1


#
# -- Define the directory structure
#    ------------------------------
RPT-DIR=rpt
SRC-DIR=src
INC-DIR=inc
OBJ-DIR=obj
BIN-DIR=bin

#
# -- Some strings needed to complete the dependency analysis
#    -------------------------------------------------------
YY-SRC=$(sort $(wildcard $(SRC-DIR)/*.yy))
YY-CC=$(subst .yy,.cc,$(YY-SRC))
YY-HH=$(subst $(SRC-DIR)/,$(INC-DIR)/,$(subst .yy,.hh,$(YY-SRC)))
YY-D=$(subst $(SRC-DIR)/,$(OBJ-DIR)/,$(subst .yy,.d,$(YY-SRC)))

HH-SRC=$(sort $(wildcard $(INC-DIR)/*.hh) $(YY-HH))

LL-SRC=$(sort $(wildcard $(SRC-DIR)/*.ll))
LL-CC=$(subst .ll,.cc,$(LL-SRC))
LL-OBJ=$(subst $(SRC-DIR)/,$(OBJ-DIR)/,$(subst .ll,.o,$(LL-SRC)))
LL-D=$(subst $(SRC-DIR)/,$(OBJ-DIR)/,$(subst .ll,.d,$(LL-SRC)))

CC-SRC=$(sort $(wildcard $(SRC-DIR)/*.cc) $(YY-CC) $(LL-CC))

ALL-SRC=$(sort $(YY-SRC) $(LL-SRC) $(wildcard $(SRC-DIR)/*.cc) $(wildcard $(INC-DIR)/*.hh))

OBJ=$(subst $(SRC-DIR)/,$(OBJ-DIR)/,$(subst .cc,.o,$(CC-SRC)))
DEP=$(subst .o,.d,$(OBJ))

RPT=$(RPT-DIR)/ast-cc.rpt

TGT=$(BIN-DIR)/ast-cc

#
# -- These are the build commands
#    ----------------------------
CFLAGS=-Wno-write-strings -Wno-unused-function
CEXTRA=-Wno-sign-compare
LIBS=-lstdc++ -lm

LEX=flex --yylineno
YAC=bison --defines=$(YY-HH) --debug --report-file=$(RPT) --report=all
GCC=gcc -c -Wall -g -Iinc $(CFLAGS)
LD=gcc


.phony: all clean install dump


#
# -- Finally, these are the recipes needed to build the actual system
#    ----------------------------------------------------------------
all: $(TGT) makefile

ifneq ($(MAKECMDGOALS),clean)
-include $(DEP)
endif

$(TGT): $(OBJ)
	echo "LINK  " $@
	mkdir -p $(BIN-DIR)
	$(LD) -o $@ $(OBJ) $(LIBS)

$(LL-OBJ): $(LL-CC)
	echo "CC    " $<
	mkdir -p $(OBJ-DIR)
	$(GCC) $(CEXTRA) -o $@ $<

$(OBJ-DIR)/%.o: $(SRC-DIR)/%.cc
	echo "CC    " $<
	mkdir -p $(OBJ-DIR)
	$(GCC) -o $@ $<

$(SRC-DIR)/%.cc: $(SRC-DIR)/%.yy
	echo "YACC  " $<
	mkdir -p $(RPT-DIR)
	$(YAC) -o $@ $<

$(SRC-DIR)/%.cc: $(SRC-DIR)/%.ll
	echo "LEX   " $<
	$(LEX) -o $@ $<

$(OBJ-DIR)/%.d: $(SRC-DIR)/%.cc
	echo "DEPEND" $(notdir $<)
	mkdir -p $(OBJ-DIR)
	$(GCC) -MM -MG $< | sed -e 's@^\(.*\)\.o:@obj/\1.d obj/\1.o:@' | sed -e 's/ parser.hh/ inc\/parser.hh/' > $@


$(YY-D) $(YY-HH) $(YY-CC): $(YY-SRC)
$(LL-D) $(LL-CC): $(LL-SRC)

clean:
	rm -f $(YY-CC)
	rm -f $(YY-HH)
	rm -f $(LL-CC)
	rm -f *~
	rm -fR $(RPT-DIR) $(OBJ-DIR) $(BIN-DIR)
	rm -f .gitignore~
	rm -f ast-nodes.hh
	echo "All cleaned up!"


install: all
	echo "INST  " $(TGT)
	mkdir -p ~/bin
	cp $(TGT) ~/bin
	
dump:
	echo "Headers " $(HH-SRC)
	echo "Scanner " $(LL-SRC)
	echo "Grammar " $(YY-SRC)
	echo "C++ Src " $(CC-SRC)
	echo "Objects " $(OBJ)
	echo "Deps    " $(DEP)
	echo "All Src " $(ALL-SRC)
		
	
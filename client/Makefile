# Каталоги
IDIR = include
CDIR = src
TCDIR = tests/unit
ODIR = bin
TODIR = bin/test
TEXDIR = tex
TEXINCDIR = $(TEXDIR)/include
DOXDIR = doxygen
UDIR = utils
DTODIR = dot

# Утилитки
MAKE2DOT = $(UDIR)/makefile2dot
MAKESIMPLE = $(UDIR)/makesimple
RE2TEX = $(UDIR)/re2tex
FSM2DOT = $(UDIR)/fsm2dot
CFLOW=cflow --level "0= "
CFLOW2DOT=$(UDIR)/cflow2dot
# Файл cflow.ignore содержит список функций,
# исключаемых из графов вызовов.
SIMPLECFLOW=grep -v -f cflow.ignore
SRC2TEX=$(UDIR)/src2tex

# Главная программа
PROG = client.elf

# Юнит тесты
TPROG = client_test.elf

# Отчёт
REPORT = report.pdf

# Компилятор С
CC = gcc
# Флаги компиляции
CFLAGS = -I$(IDIR) -Wall -Werror -pedantic -std=gnu99 -g -fPIC
TCFLAGS = -Wall -Werror -pedantic -std=gnu99 -g -fPIC
# -Werror
# Флаги сборки
LDFLAGS += $(shell autoopts-config ldflags)
LDFLAGS += -lpthread -lresolv

test_ldflags += -lcunit

# latex -> pdf
PDFLATEX = pdflatex -interaction=nonstopmode

# Файл с регулярными выражениями
# Файл с графом процесса сборки программы
Makefile_1.dot = $(DTODIR)/Makefile_1.dot

# По-умолчанию и по make all собираем и отчёт и программу.
# Отдельно отчёт собирается как make report
all: $(PROG) 

INCLUDES = $(wildcard $(IDIR)/*.h) $(IDIR)/checkoptn.h $(IDIR)/client-fsm.h
CSRC = $(wildcard $(CDIR)/*.c)
CSRC += $(CDIR)/checkoptn.c $(CDIR)/client-fsm.c

TINCLUDES = $(wildcard $(TCDIR)/*.h)
TCSRC = $(wildcard $(TCDIR)/*.c)

# Объектные файлы. Обычно, наоборот, по заданному списку объектных получают
# список исходных файлов. Но мне лень.
OBJS = $(patsubst $(CDIR)/%, $(ODIR)/%, $(patsubst %.c,%.o,$(CSRC)))

# Файлы latex
TEXS = $(wildcard $(TEXDIR)/*.tex)

# Кодогенерация: разбор параметров командной строки
$(CDIR)/checkoptn.c: $(CDIR)/checkoptn.def
	cd $(CDIR) && SHELL=/bin/sh autogen checkoptn.def
$(IDIR)/checkoptn.h:  $(CDIR)/checkoptn.def
	cd $(CDIR) && SHELL=/bin/sh autogen checkoptn.def
	mv -f $(CDIR)/checkoptn.h $(IDIR)
# Кодогенерация: конечный автомат
$(CDIR)/client-fsm.c: $(CDIR)/client.def
	cd $(CDIR) && autogen client.def
$(IDIR)/client-fsm.h: $(CDIR)/client.def
	cd $(CDIR) && autogen client.def
	mv -f $(CDIR)/client-fsm.h $(IDIR)

# Сгенерённый код компилируется особым образом
# (как того требует инструкция).
$(ODIR)/checkoptn.o: $(CDIR)/checkoptn.c
	$(CC) -c -I$(IDIR) -DTEST_CHECK_OPTS `autoopts-config cflags` -o $@  $<

# Сгенерённый код компилируется особым образом (костыли).
$(ODIR)/client-fsm.o: $(CDIR)/client-fsm.c
	$(CC) -c -I$(IDIR) -Wall -o $@ $<

$(DTODIR)/%_def.dot: $(CDIR)/%.def
	$(FSM2DOT) $< > $@

$(DTODIR)/processes.dot:
	cp tex/dot/processes.dot $(DTODIR)/processes.dot

# .dot -> _dot.tex
$(TEXINCDIR)/%_dot.tex: $(DTODIR)/%.dot
	utils/dot2tex -ftikz --autosize --crop  $< > $@

# _dot.tex -> _dot.pdf
$(TEXINCDIR)/%_dot.pdf: $(TEXINCDIR)/%_dot.tex
	$(PDFLATEX) -output-directory $(TEXINCDIR) $<

#$(TEXINCDIR)/%_re.tex: $(RE_FILE)
#	$(RE2TEX) $(RE_FILE) $(TEXINCDIR)

$(ODIR)/%.o: $(CDIR)/%.c $(INCLUDES) # $(GENINCLUDES)
	$(CC) -c $(CFLAGS) -o $@ $<

$(PROG): $(OBJS)
	$(CC) $^ $(LDFLAGS) -o $@

# Отчёт. PDFLATEX вызывается дважды для нормального 
# создания ссылок, это НЕ опечатка.
$(REPORT): $(TEXS) doxygen $(addprefix $(TEXINCDIR)/, client_def_dot.pdf Makefile_1_dot.pdf cflow01_dot.pdf checkoptn.def.tex)
	cp $(DOXDIR)/latex/*.sty ~/texmf/tex/latex/
	texhash ~/texmf 
	cd $(TEXDIR) && echo `$(PDFLATEX) report.tex` 
	cd $(TEXDIR) && echo `$(PDFLATEX) report.tex` 
	cp $(TEXDIR)/$(REPORT) ./

# Это сокращённый файл параметров программы дял отчёта, 
# берутся строки, начиная с восьмой, строки с descrip -- удаляются.
$(TEXINCDIR)/checkoptn.def.tex: $(CDIR)/checkoptn.def
	$(SRC2TEX) $< 8 | grep -v descrip > $@
# Файл simplest.mk -- это упрощёный makefile, промежуточная стадия
# для созданя графа сборки программы
simplest.mk: Makefile
	sed 's/$$(INCLUDES)//'  Makefile | $(MAKESIMPLE)  > simplest.mk

$(DTODIR)/cflow01.dot: $(addprefix $(CDIR)/, worker.c)
	$(CFLOW) $^ | $(SIMPLECFLOW) | $(CFLOW2DOT) > $@

$(DTODIR)/cflow02.dot: $(addprefix $(CDIR)/, SMTP_connection.c client-fsm.c)
	$(CFLOW) $^ | $(SIMPLECFLOW) | $(CFLOW2DOT) > $@

$(Makefile_1.dot): $(MAKE2DOT) simplest.mk
	$(MAKE2DOT) $(PROG) < simplest.mk > $(Makefile_1.dot)

# Тестирование
.PHONY: tests
tests: test_units test_memory test_style test_system

.PHONY: test_units

TOBJS = $(patsubst $(TCDIR)/%, $(TODIR)/%, $(patsubst %.c,%.o,$(TCSRC)))

$(TODIR)/%.o: $(TCDIR)/%.c $(INCLUDES) $(TINCLUDES) 
	$(CC) -c -Iinclude -I$(TCDIR)  $(TCFLAGS) -o $@ $<

TOBJS += $(OBJS)
FTOBJS = $(filter-out $(ODIR)/client_main.o $(ODIR)/logger.o $(ODIR)/worker.o $(ODIR)/master.o $(ODIR)/end_program_handler.o, $(TOBJS))

$(TPROG): $(FTOBJS)
	$(CC) $^ $(LDFLAGS) $(test_ldflags) -o $@

test_units: $(TPROG)

	

.PHONY: test_style
test_style:
	echo Сделайте сами на основе astyle.sourceforge.net или checkpatch.pl
	echo Для инженеров -- не обязательно

.PHONY: test_memory
test_memory: $(PROG)
	echo Сделайте сами на основе valgrind

.PHONY: test_system
test_system: $(PROG)
	echo Сделайте сами на основе тестирования через netcat и/или скриптов на python/perl/etc.
	#./$(PROG) -p 1025 -f tests/test01.cmds

# Документация
# .PHONY: report
report:  $(REPORT)

.PHONY: doxygen
doxygen: doxygen.cfg $(CSRC) $(INCLUDES)
	/usr/bin/doxygen doxygen.cfg
	cp $(DOXDIR)/latex/*.tex $(TEXINCDIR)

.PHONY: clean
clean:
	rm -rf $(CDIR)/checkoptn.c $(IDIR)/checkoptn.h ;\
	rm -rf $(ODIR)/*.o $(CDIR)/*~ $(IDIR)/*~ $(TEXINCDIR)/* $(DOXDIR)/* $(DTODIR)/*.dot; \
	rm simplest.mk *~; \
	rm *.elf *.pdf; \
	rm -r `find $(TEXDIR)/ -type f ! -name "*.tex"` 

.PHONY: clean_tests
clean_tests:
	rm -rf $(TODIR)/*.o



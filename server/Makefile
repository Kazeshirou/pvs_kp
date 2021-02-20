# Каталоги
IDIR = include
CDIR = src
UNITDIR = tests/unit
ODIR = bin
TEXDIR = tex
TEXINCDIR = $(TEXDIR)/include
DOXDIR = doxygen
UDIR = utils
DOTDIR = dot

# Утилитки
MAKE2DOT = $(UDIR)/makefile2dot
MAKESIMPLE = $(UDIR)/makesimple
RE2TEX = $(UDIR)/re2tex
FSM2DOT = $(UDIR)/fsm2dot
CFLOW=cflow --level "0= "
CFLOW2DOT=$(UDIR)/cflow2dot
# Файл cflow.ignore содержит список функций,front
# исключаемых из графов вызовов.
SIMPLECFLOW=grep -v -f cflow.ignore
SRC2TEX=$(UDIR)/src2tex

# Главная программа
PROG = server.elf
UNIT_TESTS = unit_tests.elf

# Отчёт
REPORT = report.pdf

# Компилятор С
CC = gcc
# Флаги компиляции
CFLAGS = -std=c99  -I$(IDIR) -Wall -Werror -pedantic

ifdef DEBUG
CFLAGS += -g
endif

ifdef OPT
CFLAGS += -O0
endif

# -Werror
# Флаги сборки
LDFLAGS +=  $(shell autoopts-config ldflags)
LDFLAGS += -lpthread -lpcre

# latex -> pdf
PDFLATEX = pdflatex -interaction=nonstopmode

# Файл с регулярными выражениями
RE_FILE = $(IDIR)/re_definitions.h
# Файл с графом процесса сборки программы
Makefile_1.dot = $(DOTDIR)/Makefile_1.dot

# По-умолчанию и по make all собираем и отчёт и программу.
# Отдельно отчёт собирается как make report
all: $(PROG)

# Добавляются имена сгенерённых файлов.
# GENINCLUDES = $(IDIR)/checkoptn.h $(IDIR)/server-fsm.h

INCLUDES = $(wildcard $(IDIR)/*.h) $(IDIR)/checkoptn.h $(IDIR)/client-fsm.h
# $(wildcard $(CDIR)/*.c)
CSRC = $(wildcard $(CDIR)/*.c)
CSRC += $(CDIR)/checkoptn.c $(CDIR)/client-fsm.c
UNIT_TESTS_CSRV = $(wildcard $(UNITDIR)/*.c)
UNIT_TESTS_CSRV += $(shell find $(CDIR) -name '*.c' -not -path '$(CDIR)/server_main.c')


# Объектные файлы. Обычно, наоборот, по заданному списку объектных получают
# список исходных файлов. ЕНо мне лень.
OBJS = $(patsubst $(CDIR)/%, $(ODIR)/%, $(patsubst %.c,%.o,$(CSRC)))
UNIT_TESTS_OBJS = $(patsubst $(UNITDIR)/%, $(ODIR)/%, $(patsubst $(CDIR)/%, $(ODIR)/%, $(patsubst %.c,%.o,$(UNIT_TESTS_CSRV))))

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
# $(ODIR)/server-fsm.o: $(CDIR)/server-fsm.c
# 	$(CC) -I$(IDIR) -Wall -o $@ -c $<

$(DOTDIR)/%_def.dot: $(CDIR)/%.def
	$(FSM2DOT) $< > $@

# .dot -> _dot.tex
$(TEXINCDIR)/%_dot.tex: $(DOTDIR)/%.dot
	utils/dot2tex -ftikz --autosize --crop  $< > $@

# _dot.tex -> _dot.pdf
$(TEXINCDIR)/%_dot.pdf: $(TEXINCDIR)/%_dot.tex
	$(PDFLATEX) -output-directory $(TEXINCDIR) $<

$(TEXINCDIR)/%_re.tex: $(RE_FILE)
	$(RE2TEX) $(RE_FILE) $(TEXINCDIR)

$(ODIR)/%.o: $(UNITDIR)/%.c $(INCLUDES) # $(GENINCLUDES)
	$(CC) -c $(CFLAGS) -o $@ $<

$(ODIR)/%.o: $(CDIR)/%.c $(INCLUDES) # $(GENINCLUDES)
	$(CC) -c $(CFLAGS) -o $@ $<

$(PROG): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(UNIT_TESTS): $(UNIT_TESTS_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) -lcunit

# Отчёт. PDFLATEX вызывается дважды для нормального 
# создания ссылок, это НЕ опечатка.
$(REPORT): $(TEXS) doxygen $(addprefix $(TEXINCDIR)/, client_def_dot.pdf Makefile_1_dot.pdf re_definitions_re.tex cflow01_dot.pdf  checkoptn.def.tex)
	cp $(DOXDIR)/latex/*.sty ~/texmf/tex/latex/
	texhash ~/texmf 
	cd $(TEXDIR) && echo `$(PDFLATEX) report.tex` 
	cd $(TEXDIR) && echo `$(PDFLATEX) report.tex` 
	cp $(TEXDIR)/$(REPORT) ./

# Это сокращённый файл параметров программы дял отчёта, 
# берутся строки, начиная с восьмой, строки с descrip -- удаляются.
$(TEXINCDIR)/checkoptn.def.tex: $(CDIR)/checkoptn.def
	$(SRC2TEX) $< 5 | grep -v descrip > $@
# Файл simplest.mk -- это упрощёный makefile, промежуточная стадия
# для созданя графа сборки программы
simplest.mk: Makefile
	sed 's/$$(INCLUDES)//'  Makefile | $(MAKESIMPLE)  > simplest.mk

$(DOTDIR)/cflow01.dot: $(addprefix $(CDIR)/, server_main.c smtp_server.c server_info.c)
	$(CFLOW) $^ | $(SIMPLECFLOW) | $(CFLOW2DOT) > $@

# $(DOTDIR)/cflow02.dot: $(addprefix $(CDIR)/, server-cmd.c server-parse.c server-fsm.c)
# 	$(CFLOW) $^ | $(SIMPLECFLOW) | $(CFLOW2DOT) > $@

$(Makefile_1.dot): $(MAKE2DOT) simplest.mk
	$(MAKE2DOT) $(PROG) < simplest.mk > $(Makefile_1.dot)

# Тестирование
.PHONY: tests
tests: test_style test_units test_system test_memory

.PHONY: test_units
test_units: $(UNIT_TESTS)
	./$(UNIT_TESTS)

.PHONY: test_style
test_style:
	./tests/style_check.sh

.PHONY: test_memory
test_memory:  test_memory_unit test_memory_system

.PHONY: test_memory_system
test_memory_system: $(PROG)
	./tests/system_test.py ./tests/valgrind_check.sh

.PHONY: test_memory_unit
test_memory_unit: $(UNIT_TESTS)
	valgrind --leak-check=full \
		 --error-exitcode=1 \
		 --show-leak-kinds=all \
		 --track-origins=yes \
		 --exit-on-first-error=yes \
		 --verbose \
		 --log-file=tests/tmp/valgrind-out-unit.txt \
		 $(UNIT_TESTS); 

.PHONY: test_system
test_system: $(PROG)
	./tests/system_test.py

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
	rm -rf $(ODIR)/*.o $(TEXINCDIR)/* $(DOXDIR)/* $(DOTDIR)/*.dot; \
	rm simplest.mk; \
	rm *.elf report.pdf tex/report.pdf ; \
	rm -rf tests/tmp/*



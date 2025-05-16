ENVTXT ?=
SIZE ?= 0x40000 # 256KB env.bin as default

ifneq ($(MAKECMDGOALS),clean)
ifeq ($(ENVTXT),)
$(error please specify plain text env by argument "ENVTXT=")
endif
endif

ABSPATH_ENV_TXT=$(shell cd $(PWD);readlink -f $(ENVTXT))
ENV_SIZE := $(SIZE)
ENV_BIN := $(CURDIR)/env.bin
BURN_XML := $(CURDIR)/burn_table.xml

.PHONY: all
all: env burnxml

.PHONY: env
env: $(ENV_BIN)

.PHONY: burnxml
burnxml: $(BURN_XML)

$(ENV_BIN): FORCE
	@mkenvimage -s $(ENV_SIZE) -o $(ENV_BIN) $(ABSPATH_ENV_TXT)
	@chmod +r $(ENV_BIN)
	@echo "Generate $(notdir $(ENV_BIN))"

$(BURN_XML): FORCE
	@python3 scripts/env2burn.py $(ABSPATH_ENV_TXT) $(BURN_XML)
	@echo "Generate $(notdir $(BURN_XML))"

.PHONY: clean
clean:
	rm -f $(ENV_BIN)
	rm -f $(BURN_XML)

PHONY: FORCE
FORCE:

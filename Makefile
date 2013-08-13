SUB_PROJECTS=$(shell ls -d */)
clean_SUB_PROJECTS=$(addprefix clean_,$(SUB_PROJECTS))
.PHONY: force

all: $(SUB_PROJECTS)
$(SUB_PROJECTS): force
	make -C $@

clean: $(clean_SUB_PROJECTS)
$(clean_SUB_PROJECTS): force
	make -C $(patsubst clean_%,%,$@) clean
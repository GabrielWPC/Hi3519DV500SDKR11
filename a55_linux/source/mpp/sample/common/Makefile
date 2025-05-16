param_file=$(realpath $(dir $(firstword $(MAKEFILE_LIST))))/../Makefile.param
include $(param_file)
DEF_FLAGS := _GNU_SOURCE
.PHONY : clean all
all:$(COMM_OBJ)

clean:
	@rm $(COMM_OBJ) -f

%.o:%.c
	@$(CC) $(COMM_INC) $(CFLAGS) -c $< -o $@

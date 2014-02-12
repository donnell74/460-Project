CFLAGS = -C 
DIRS = Server Client

build:
	for each_dir in $(DIRS); do \
		$(MAKE) $(CFLAGS) $$each_dir build; \
	done


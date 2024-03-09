COMPILE_COMMANDS=$(CURDIR)/compile_commands.json

$(COMPILE_COMMANDS):
	compiledb make all tests

run-clang-format:
	find $(CURDIR) | grep "\.[h|c]$$" | xargs -n 1 clang-format -n

run-clang-reformat:
	find $(CURDIR) | grep "\.[h|c]$$" | xargs -n 1 clang-format -i

run-clang-tidy: $(COMPILE_COMMANDS)
	find $(CURDIR) | grep "\.[h|c]$$" | xargs -n 1 clang-tidy -p $^ --config-file=$(CURDIR)/.clang-tidy

run-cppcheck: $(COMPILE_COMMANDS)
	find $(CURDIR) | grep "\.[h|c]$$" | xargs -n 1 cppcheck --enable=all --std=c11 --suppress=missingIncludeSystem --project=$^

PHONY += \
	run-clang-format \
	run-clang-reformat \
	run-clang-tidy \
	run-cppcheck

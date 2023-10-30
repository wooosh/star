.PHONY: docs deps

docs: deps
	jpm deps
	jpm build
	janet build.janet

deps:
	star_build_deps
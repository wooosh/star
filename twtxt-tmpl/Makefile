.POSIX:
.PHONY: star-dep

twtxt-tmpl: main.go
	go build

star-dep: twtxt-tmpl
	cp twtxt-tmpl "${STAR_ROOT}/bin"
.PHONY: fonts user-install user-uninstall clean star-dep

SRCS = deserted.fontfamily \
			 logic.png space.png symbols.png symbols-bold.png \
       deserted-letters.png beached-letters.png stranded-letters.png grounded-letters.png

fonts: build/fonts.stamp

build/fonts.stamp: png2bdf makefont $(SRCS)
	mkdir -p build
	./makefont deserted.fontfamily
	touch build/fonts.stamp

user-install: fonts
	mkdir -p "${HOME}/.fonts/deserted"
	cp build/*.bdf "${HOME}/.fonts/deserted"
	if [ "$(NO_TTF)" != 1 ]; then \
		cp build/*.ttf "${HOME}/.fonts/deserted"; \
	fi
	fc-cache -fv

user-uninstall:
	rm -rf "$HOME/.fonts/deserted"

clean:
	rm -rf build png2bdf

png2bdf: png2bdf.cpp

star-dep: fonts
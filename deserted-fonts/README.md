**NOTICE**: all fonts other than deserted are WIP

***

Requires:

* `make`
* `g++` or `clang++`
* For TTF fonts:
  * `python3`
  * `fontforge`
  * `potrace`

***

Installation:

```
git clone https://github.com/wooosh/star
cd star/deserted-fonts
# if the TTF font dependencies are installed:
make user-install
# otherwise
make user-install NO_TTF=1
```

***

Usage:

The fonts use the following font names in fontconfig

* `deserted`, `deserted_ttf`
* `stranded`, `stranded_ttf`
* `grounded`, `grounded_ttf`
* `beached`, `beached_ttf`

The fonts should be used at size 12, which should be the default size.

If the fonts do not look right, you are likely using the wrong size. If you have some form of display scaling set up, it may need to be used at a size other than 12.

***

Licensing

* US copyright law does not protect bitmap fonts
* The files contained in this directory (bitmap fonts, scalable fonts, images, and software) are released into the public domain to the extent allowed by law under the CC0 1.0 License in all jurisdictions, including those that protect bitmap fonts under copyright law
* If you use this in any form of project (game, website, software, etc), I would greatly appreciate it if you credited me and [opened an issue on this repo](https://github.com/wooosh/star/issues/new) with a link or description of your project.
* The following files have been included from external sources, which have different licensing requirements:
  * `mkttf.py` is from [Tblue/mkttf](https://github.com/Tblue/mkttf) and is licensed as BSD 3-Clause.
  * `stb_image.h` is from [nothings/stb](https://github.com/nothings/stb) is dual licensed as MIT or Unlicense.

`LICENSE.md` contains licensing information for both the fonts and software.

`RELEASE-LICENSE.md` contains licensing information for only the fonts.
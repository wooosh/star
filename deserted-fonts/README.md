Notice: all fonts other than deserted are WIP

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
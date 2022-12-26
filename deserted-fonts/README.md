Requires:

* `make`
* `g++` or `clang++`
* For TTF fonts:
  * `python3`
  * `fontforge`
  * `potrace`

Installation:

```
git clone https://github.com/wooosh/star
cd star/deserted-fonts
# if the TTF font dependencies are installed:
make user-install
# otherwise
make user-install NO_TTF=1
```
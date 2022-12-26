#include <ios>
#include <iostream>
#include <iomanip>
#include <codecvt>

// TODO: take pbm image

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// arguments: png glyphwidth charorder
int main(int argc, char **argv) {
  if (argc != 4) {
    std::cerr << "incorrect # of arguments\n";
    std::abort();
  }

  const char *filename = argv[1];
  int charwidth = std::stoi(argv[2]);
  std::string chars_narrow = argv[3];
  std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> utf8conv;
  std::wstring chars = utf8conv.from_bytes(chars_narrow);

  int img_w, img_h, num_channels;
  uint32_t *img = (uint32_t *) stbi_load(filename, &img_w, &img_h, &num_channels, 4);
  if (img == NULL) {
    std::cerr << "cannot read image\n";
    std::abort();
  }

  for (size_t i=0; i<chars.size(); i++) {
    std::cout << std::hex << "STARTCHAR U+00" << (int) chars[i] << std::dec << "\n"
              << "ENCODING " << (int) chars[i] << "\n"
              << "SWIDTH 600 0\n" // TODO: pixels = (scalable_width / 1000) * (resolution / 72)
              << "DWIDTH " << charwidth << " 0\n"
              << "BBX " << charwidth << " " << img_h << " 0 -2\n"
              << "BITMAP\n";

    std::cout << std::uppercase << std::hex;
    for (size_t y = 0; y < img_h; y++) {
      int buffer = 0;
      int buf_len = 0;
      for (size_t x = 0; x < charwidth; x++) {
        uint32_t pixel = img[y*img_w + i*charwidth + x];
        bool bit = (pixel & 0x00FFFFFF) == 0;
        buffer |= bit;
        buf_len++;
        if (buf_len == 8) {
          std::cout << std::setfill('0') << std::setw(2) << buffer;
          buffer = 0;
          buf_len = 0;
        }
        buffer <<= 1;
      }
      if (buf_len > 0) {
        std::cout << std::setfill('0') << std::setw(2) << buffer;
      }
      std::cout << "\n";
    }
    std::cout << std::dec;
    std::cout << "ENDCHAR\n";
  }
}

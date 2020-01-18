//
// Created by psi on 2020/01/18.
//

#pragma once

#include <aom/aom_image.h>
#include <vector>

class Image;
class ImageConverter {
public:
  ImageConverter() = delete;
  ImageConverter(ImageConverter const&) = delete;
  ImageConverter(ImageConverter&&) = delete;
  ImageConverter& operator=(ImageConverter const&) = delete;
  ImageConverter& operator=(ImageConverter&&) = delete;

private:
  size_t const width_;
  size_t const height_;
  Image& src_;
  aom_image& dst_;
public:
  explicit ImageConverter(Image& src, aom_image& dst);
  void convert();
private:
  void convertFromBGRA(std::vector<uint8_t> const& bgra, size_t stride);
};



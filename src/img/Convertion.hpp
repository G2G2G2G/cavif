//
// Created by psi on 2020/01/18.
//

#include <stdexcept>
#include <fmt/format.h>
#include "avif/img/Image.hpp"
#include "avif/img/Conversion.hpp"

using MatrixType = avif::av1::SequenceHeader::ColorConfig::MatrixCoefficients;

namespace detail {

template <MatrixType matrixType, uint8_t rgbBits, uint8_t yuvBits, bool fromMonoRGB, bool isFullRange>
void convertImage(avif::img::Image<rgbBits>& src, aom_image& dst) {
  if(dst.monochrome) {
    avif::img::FromRGB<matrixType, rgbBits, yuvBits, fromMonoRGB, isFullRange>().toI400(src,
                                                                                        dst.planes[0], dst.stride[0]);
  } else {
    switch (dst.fmt) {
      case AOM_IMG_FMT_I420:
      case AOM_IMG_FMT_I42016:
        avif::img::FromRGB<matrixType, rgbBits, yuvBits, fromMonoRGB, isFullRange>().toI420(src,
                                                                                            dst.planes[0], dst.stride[0],
                                                                                            dst.planes[1], dst.stride[1],
                                                                                            dst.planes[2], dst.stride[2]);
        break;
      case AOM_IMG_FMT_I422:
      case AOM_IMG_FMT_I42216:
        avif::img::FromRGB<matrixType, rgbBits, yuvBits, fromMonoRGB, isFullRange>().toI422(src,
                                                                                            dst.planes[0], dst.stride[0],
                                                                                            dst.planes[1], dst.stride[1],
                                                                                            dst.planes[2], dst.stride[2]);
        break;
      case AOM_IMG_FMT_I444:
      case AOM_IMG_FMT_I44416:
        avif::img::FromRGB<matrixType, rgbBits, yuvBits, fromMonoRGB, isFullRange>().toI444(src,
                                                                                            dst.planes[0], dst.stride[0],
                                                                                            dst.planes[1], dst.stride[1],
                                                                                            dst.planes[2], dst.stride[2]);
        break;
      default:
        throw std::invalid_argument(fmt::format("Unsupported image format: {:08x}", dst.fmt));
    }
  }
}

template <MatrixType matrixType, uint8_t rgbBits, uint8_t yuvBits, bool fromMonoRGB>
void convertImage(avif::img::Image<rgbBits>& src, aom_image& dst) {
  if(dst.range == AOM_CR_FULL_RANGE) {
    convertImage<matrixType, rgbBits, yuvBits, fromMonoRGB, true>(src, dst);
  } else {
    convertImage<matrixType, rgbBits, yuvBits, fromMonoRGB, false>(src, dst);
  }
}

template <MatrixType matrixType, uint8_t rgbBits, uint8_t yuvBits>
void convertImage(avif::img::Image<rgbBits>& src, aom_image& dst) {
  if(src.isMonochrome()) {
    convertImage<matrixType, rgbBits, yuvBits, true>(src, dst);
  } else {
    convertImage<matrixType, rgbBits, yuvBits, false>(src, dst);
  }
}

template <MatrixType matrixType, uint8_t rgbBits, uint8_t yuvBits, bool isFullRange>
void convertAlpha(avif::img::Image<rgbBits>& src, aom_image& dst) {
  if (dst.monochrome) {
    avif::img::FromAlpha<matrixType, rgbBits, yuvBits, isFullRange>().toI400(src,
                                                                             dst.planes[0], dst.stride[0]);
  } else {
    throw std::invalid_argument("Alpha image should be monochrome. Please add --monochrome option.");
  }
}

template <MatrixType matrixType, uint8_t rgbBits, uint8_t yuvBits>
void convertAlpha(avif::img::Image<rgbBits>& src, aom_image& dst) {
  if(dst.range == AOM_CR_FULL_RANGE) {
    convertAlpha<matrixType, rgbBits, yuvBits, true>(src, dst);
  } else {
    convertAlpha<matrixType, rgbBits, yuvBits, false>(src, dst);
  }
}

}

template <MatrixType matrixType, Config::EncodeTarget target, uint8_t rgbBits, uint8_t yuvBits>
void convert(avif::img::Image<rgbBits>& src, aom_image& dst) {
  switch (target) {
    case Config::EncodeTarget::Image:
      detail::convertImage<matrixType, rgbBits, yuvBits>(src, dst);
      break;
    case Config::EncodeTarget::Alpha:
      detail::convertAlpha<matrixType, rgbBits, yuvBits>(src, dst);
      break;
    default:
      throw std::invalid_argument(fmt::format("Unsupported EncodeTarget: {}", target));
  }
}

template <MatrixType matrixType, Config::EncodeTarget target, size_t rgbBits>
void convert(avif::img::Image<rgbBits>& src, aom_image& dst) {
  switch (dst.bit_depth) {
    case 8:
      convert<matrixType, target, rgbBits, 8>(src, dst);
      break;
    case 10:
      convert<matrixType, target, rgbBits, 10>(src, dst);
      break;
    case 12:
      convert<matrixType, target, rgbBits, 12>(src, dst);
      break;
    default:
      throw std::invalid_argument(fmt::format("Unsupported YUV bit-depth: {}", dst.bit_depth));
  }
}

template <MatrixType matrixType, size_t rgbBits>
void convert(Config& config, avif::img::Image<rgbBits>& src, aom_image& dst) {
  switch (config.encodeTarget) {
    case Config::EncodeTarget::Image:
      convert<matrixType, Config::EncodeTarget::Image, rgbBits>(src, dst);
      break;
    case Config::EncodeTarget::Alpha:
      convert<matrixType, Config::EncodeTarget::Alpha, rgbBits>(src, dst);
      break;
    default:
      assert(false && "[BUG] Unkown encoder target.");
  }
}

template <size_t rgbBits>
void convert(Config& config, avif::img::Image<rgbBits>& src, aom_image& dst) {
  aom_img_fmt_t const pixFmt = config.codec.g_bit_depth == 8 ?
        config.pixFmt :
        static_cast<aom_img_fmt_t>(config.pixFmt | static_cast<unsigned int>(AOM_IMG_FMT_HIGHBITDEPTH));
  aom_img_alloc(&dst, pixFmt, src.width(), src.height(), 1);
  dst.range = config.fullColorRange ? AOM_CR_FULL_RANGE : AOM_CR_STUDIO_RANGE;
  dst.monochrome = config.codec.monochrome ? 1 : 0;
  dst.bit_depth = config.codec.g_bit_depth;
  switch (static_cast<MatrixType>(config.matrixCoefficients)) {
    case MatrixType::MC_IDENTITY:
      convert<MatrixType::MC_IDENTITY, rgbBits>(config, src, dst);
      break;
    case MatrixType::MC_BT_709:
      convert<MatrixType::MC_BT_709, rgbBits>(config, src, dst);
      break;
    case MatrixType::MC_FCC:
      convert<MatrixType::MC_FCC, rgbBits>(config, src, dst);
      break;
    case MatrixType::MC_BT_470_B_G:
      convert<MatrixType::MC_BT_470_B_G, rgbBits>(config, src, dst);
      break;
    case MatrixType::MC_BT_601:
      convert<MatrixType::MC_BT_601, rgbBits>(config, src, dst);
      break;
    case MatrixType::MC_SMPTE_240:
      convert<MatrixType::MC_SMPTE_240, rgbBits>(config, src, dst);
      break;
    case MatrixType::MC_SMPTE_YCGCO:
      convert<MatrixType::MC_SMPTE_YCGCO, rgbBits>(config, src, dst);
      break;
    case MatrixType::MC_UNSPECIFIED:
    case MatrixType::MC_BT_2020_NCL:
      convert<MatrixType::MC_BT_2020_NCL, rgbBits>(config, src, dst);
      break;
    case MatrixType::MC_BT_2020_CL:
      convert<MatrixType::MC_BT_2020_CL, rgbBits>(config, src, dst);
      break;
    case MatrixType::MC_SMPTE_2085:
      convert<MatrixType::MC_SMPTE_2085, rgbBits>(config, src, dst);
      break;
    case MatrixType::MC_CHROMAT_NCL:
      convert<MatrixType::MC_CHROMAT_NCL, rgbBits>(config, src, dst);
      break;
    case MatrixType::MC_CHROMAT_CL:
      convert<MatrixType::MC_CHROMAT_CL, rgbBits>(config, src, dst);
      break;
    case MatrixType::MC_ICTCP:
      convert<MatrixType::MC_ICTCP, rgbBits>(config, src, dst);
      break;
    default:
      assert(false && "Unknown matrix coefficients");
  }
}

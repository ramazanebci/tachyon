#ifndef TACHYON_MATH_ELLIPTIC_CURVES_SHORT_WEIERSTRASS_SW_CURVE_GPU_H_
#define TACHYON_MATH_ELLIPTIC_CURVES_SHORT_WEIERSTRASS_SW_CURVE_GPU_H_

#include "tachyon/math/elliptic_curves/short_weierstrass/sw_curve_base.h"
#include "tachyon/math/elliptic_curves/short_weierstrass/sw_curve_config_traits.h"
#include "tachyon/math/geometry/point2.h"

namespace tachyon::math {

template <typename _Config>
class SWCurveGpu : public SWCurveBase<SWCurveGpu<_Config>> {
 public:
  using Config = _Config;
  using BaseField = typename Config::BaseField;
  using ScalarField = typename Config::ScalarField;
  using AffinePointTy = AffinePoint<SWCurveGpu<Config>>;
  using ProjectivePointTy = ProjectivePoint<SWCurveGpu<Config>>;
  using JacobianPointTy = JacobianPoint<SWCurveGpu<Config>>;
  using PointXYZZTy = PointXYZZ<SWCurveGpu<Config>>;

  constexpr static bool kIsSWCurve = true;

  constexpr static BaseField A() { return BaseField::FromMontgomery(GetA()); }

  constexpr static BaseField B() { return BaseField::FromMontgomery(GetB()); }

  constexpr static JacobianPointTy Generator() {
    return JacobianPointTy(BaseField::FromMontgomery(GetGenerator().x),
                           BaseField::FromMontgomery(GetGenerator().y),
                           BaseField::One());
  }

  static void Init() {
    // Do nothing.
  }

 private:
  constexpr static auto GetA() { return Config::kA; }

  constexpr static auto GetB() { return Config::kB; }

  constexpr static auto GetGenerator() { return Config::kGenerator; }
};

template <typename Config>
struct SWCurveConfigTraits<SWCurveGpu<Config>> {
  using BaseField = typename Config::BaseField;
  using ScalarField = typename Config::ScalarField;
  using AffinePointTy = AffinePoint<SWCurveGpu<Config>>;
  using ProjectivePointTy = ProjectivePoint<SWCurveGpu<Config>>;
  using JacobianPointTy = JacobianPoint<SWCurveGpu<Config>>;
  using PointXYZZTy = PointXYZZ<SWCurveGpu<Config>>;
};

}  // namespace tachyon::math

#endif  // TACHYON_MATH_ELLIPTIC_CURVES_SHORT_WEIERSTRASS_SW_CURVE_GPU_H_
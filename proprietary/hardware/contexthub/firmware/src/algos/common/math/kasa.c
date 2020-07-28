#include "common/math/kasa.h"

#include <stdint.h>
#include <sys/types.h>

#include "common/math/mat.h"

void kasaReset(struct KasaFit *kasa) {
  kasa->acc_x = kasa->acc_y = kasa->acc_z = kasa->acc_w = 0.0f;
  kasa->acc_xx = kasa->acc_xy = kasa->acc_xz = kasa->acc_xw = 0.0f;
  kasa->acc_yy = kasa->acc_yz = kasa->acc_yw = 0.0f;
  kasa->acc_zz = kasa->acc_zw = 0.0f;
  kasa->nsamples = 0;
}

void kasaInit(struct KasaFit *kasa) { kasaReset(kasa); }

void kasaAccumulate(struct KasaFit *kasa, float x, float y, float z) {
  float w = x * x + y * y + z * z;

  kasa->acc_x += x;
  kasa->acc_y += y;
  kasa->acc_z += z;
  kasa->acc_w += w;

  kasa->acc_xx += x * x;
  kasa->acc_xy += x * y;
  kasa->acc_xz += x * z;
  kasa->acc_xw += x * w;

  kasa->acc_yy += y * y;
  kasa->acc_yz += y * z;
  kasa->acc_yw += y * w;

  kasa->acc_zz += z * z;
  kasa->acc_zw += z * w;

  kasa->nsamples += 1;
}

bool kasaNormalize(struct KasaFit *kasa) {
  if (kasa->nsamples == 0) {
    return false;
  }

  float inv = 1.0f / kasa->nsamples;

  kasa->acc_x *= inv;
  kasa->acc_y *= inv;
  kasa->acc_z *= inv;
  kasa->acc_w *= inv;

  kasa->acc_xx *= inv;
  kasa->acc_xy *= inv;
  kasa->acc_xz *= inv;
  kasa->acc_xw *= inv;

  kasa->acc_yy *= inv;
  kasa->acc_yz *= inv;
  kasa->acc_yw *= inv;

  kasa->acc_zz *= inv;
  kasa->acc_zw *= inv;

  return true;
}

int kasaFit(struct KasaFit *kasa, struct Vec3 *bias, float *radius,
            float max_fit, float min_fit) {
  //    A    *   out   =    b
  // (4 x 4)   (4 x 1)   (4 x 1)
  struct Mat44 A;
  A.elem[0][0] = kasa->acc_xx;
  A.elem[0][1] = kasa->acc_xy;
  A.elem[0][2] = kasa->acc_xz;
  A.elem[0][3] = kasa->acc_x;
  A.elem[1][0] = kasa->acc_xy;
  A.elem[1][1] = kasa->acc_yy;
  A.elem[1][2] = kasa->acc_yz;
  A.elem[1][3] = kasa->acc_y;
  A.elem[2][0] = kasa->acc_xz;
  A.elem[2][1] = kasa->acc_yz;
  A.elem[2][2] = kasa->acc_zz;
  A.elem[2][3] = kasa->acc_z;
  A.elem[3][0] = kasa->acc_x;
  A.elem[3][1] = kasa->acc_y;
  A.elem[3][2] = kasa->acc_z;
  A.elem[3][3] = 1.0f;

  struct Vec4 b;
  initVec4(&b, -kasa->acc_xw, -kasa->acc_yw, -kasa->acc_zw, -kasa->acc_w);

  struct Size4 pivot;
  mat44DecomposeLup(&A, &pivot);

  struct Vec4 out;
  mat44Solve(&A, &out, &b, &pivot);

  // sphere: (x - xc)^2 + (y - yc)^2 + (z - zc)^2 = r^2
  //
  // xc = -out[0] / 2, yc = -out[1] / 2, zc = -out[2] / 2
  // r = sqrt(xc^2 + yc^2 + zc^2 - out[3])

  struct Vec3 v;
  initVec3(&v, out.x, out.y, out.z);
  vec3ScalarMul(&v, -0.5f);

  float r_square = vec3Dot(&v, &v) - out.w;
  float r = (r_square > 0) ? sqrtf(r_square) : 0;

  initVec3(bias, v.x, v.y, v.z);
  *radius = r;

  int success = 0;
  if (r > min_fit && r < max_fit) {
    success = 1;
  }

  return success;
}

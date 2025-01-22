#include "ekey.h"
#include "qmc2.h"

namespace QMC2 {

std::unique_ptr<QMC2_Base> Create(std::string_view ekey) {
  auto key = decrypt_ekey(ekey);
  auto key_len = key.size();
  if (key_len == 0) {
    return nullptr;
  }

  if (key_len < 300) {
    return std::make_unique<QMC2_MAP>(std::span(key));
  } else {
    return std::make_unique<QMC2_RC4>(std::span(key));
  }
}

} // namespace QMC2

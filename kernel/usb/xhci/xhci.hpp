/**
 * xHCI ホストコントローラ制御
 */

#pragma once

#include "error.hpp"

namespace usb::xhci{
  class Controller{
    public:
      Controller(uintptr_t mmio_base);

    private:
      const uintptr_t mmio_base_;
      
  };
}
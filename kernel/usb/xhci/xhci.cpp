#include "xhci.hpp"

namespace usb::xhci{
  Controller::Controller(uintptr_t mmio_base)
    : mmio_base_{mmio_base}{
  }
  
}
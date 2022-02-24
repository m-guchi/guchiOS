/**
 * カーネル本体
 */

#include <cstdint>
#include <cstddef>
#include <cstdio>

// #include <numeric>
// #include <vector>

#include "frame_buffer_config.hpp"
#include "graphics.hpp"
// #include "mouse.hpp"
#include "font.hpp"
#include "console.hpp"
#include "pci.hpp"
#include "logger.hpp"
// #include "usb/memory.hpp"
// #include "usb/device.hpp"
// #include "usb/classdriver/mouse.hpp"
// #include "usb/xhci/xhci.hpp"
// #include "usb/xhci/trb.hpp"


// void* operator new(size_t size, void* buf){
//   return buf;
// }
void operator delete(void* obj) noexcept{
}


const int kMouseCursorWidth = 15;
const int kMouseCursorHeight = 24;
const char mouse_cursor_shape[kMouseCursorHeight][kMouseCursorWidth+1] = {
  "@              ",
  "@@             ",
  "@.@            ",
  "@..@           ",
  "@...@          ",
  "@....@         ",
  "@.....@        ",
  "@......@       ",
  "@.......@      ",
  "@........@     ",
  "@.........@    ",
  "@..........@   ",
  "@...........@  ",
  "@............@ ",
  "@......@@@@@@@@",
  "@......@       ",
  "@....@@.@      ",
  "@...@ @.@      ",
  "@..@   @.@     ",
  "@.@    @.@     ",
  "@@      @.@    ",
  "@       @.@    ",
  "         @.@   ",
  "         @@@   ",
};


char pixel_writer_buf[sizeof(RGBResv8BitPerColorPixelWriter)];
PixelWriter* pixel_writer;


char console_buf[sizeof(Console)];
Console* console;

int printk(const char* format, ...){
  va_list ap;
  int result;
  char s[1024];

  va_start(ap, format);
  result = vsprintf(s, format, ap);
  va_end(ap);

  console->PutString(s);
  return result;
}

extern "C" void KernelMain(const FrameBufferConfig& frame_buffer_config){
  switch(frame_buffer_config.pixel_format){
    case kPixelRGBResv8BitPerColor:
      pixel_writer = new(pixel_writer_buf)
        RGBResv8BitPerColorPixelWriter{frame_buffer_config};
      break;
    case kPixelBGRResv8BitPerColor:
      pixel_writer = new(pixel_writer_buf)
        BGRResv8BitPerColorPixelWriter{frame_buffer_config};
      break;
  }

  for (int x=0; x<frame_buffer_config.horizontal_resolution; x++){
    for (int y=0; y<frame_buffer_config.vertical_resolution; y++){
      pixel_writer->Write(x, y, {0, 0, 0});
    }
  }
  
  console = new(console_buf) Console{*pixel_writer, {255, 255, 255}, {0, 0, 0}};

  printk("Welcome to GuchiOS!!\n");
  SetLogLevel(kDebug);

  for(int dy=0;dy<kMouseCursorHeight;++dy){
    for(int dx=0;dx<kMouseCursorWidth;++dx){
      if(mouse_cursor_shape[dy][dx] == '@'){
        pixel_writer->Write(200+dx, 100+dy, {255,255,255});
      }else if(mouse_cursor_shape[dy][dx] == '.'){
        pixel_writer->Write(200+dx, 100+dy, {0,0,0});
      }
    }
  }

  auto err = pci::ScanAllBus();
  Log(kDebug, "ScanAllBus: %s, device_num=%d\n", err.Name(), pci::num_device);

  for (int i = 0; i < pci::num_device; ++i) {
    const auto& dev = pci::devices[i];
    auto vendor_id = pci::ReadVendorId(dev);
    auto class_code = pci::ReadClassCode(dev.bus, dev.device, dev.function);
    Log(kDebug, "%d.%d.%d: vend %04x, class %08x, head %02x\n",
        dev.bus, dev.device, dev.function,
        vendor_id, class_code, dev.header_type);
  }


  pci::Device* xhc_dev = nullptr;
  for(int i=0; i<pci::num_device;++i){
    if(pci::devices[i].class_code.Match(0x0cu, 0x03u, 0x30u)){
      xhc_dev = &pci::devices[i];
      if(0x8086 == pci::ReadVendorId(*xhc_dev)){ //Intel製を優先
        break;
      }
    }
  }
  if(xhc_dev){
    Log(kInfo, "xHC has been found: %d.%d.%d\n", xhc_dev->bus, xhc_dev->device, xhc_dev->function);
  }

  // const WithError<uint64_t> xhc_bar = pci::ReadBar(*xhc_dev, 0);
  // const uint64_t xhc_mmio_base = xhc_bar.value & ~static_cast<uint64_t>(0xf);

  
  while(1) __asm__("hlt");
}
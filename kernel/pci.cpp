/**
 * PCI関連プログラム
 * バス(0-255), デバイス(0-31), ファンクション(0-7)
 */

#include "pci.hpp"
#include "asmfunc.h"

namespace {
  using namespace pci;

  //CONFIG_ADDRESS 用の値を生成
  uint32_t MakeAddress(uint8_t bus, uint8_t device, uint8_t function, uint8_t reg_addr){
    auto shl = [](uint32_t x, unsigned int bits){ return x << bits; };
    
    return shl(1,31) | shl(bus,16) | shl(device,11) | shl(function,8) | (reg_addr & 0xfcu);
  }


  //devices に追加
  Error AddDevice(const Device& device){
    if(num_device == devices.size()){
      return Error::kFull;
    }

    devices[num_device] = device;
    ++num_device;
    return Error::kSuccess;
  }

  Error ScanBus(uint8_t bus);

  //指定ファンクションをデバイスに追加
  //PCI-PCIブリッジであれば、再帰的にScanBusを実行
  Error ScanFunction(uint8_t bus, uint8_t device, uint8_t function){
    auto class_code = ReadClassCode(bus, device, function);
    auto header_type = ReadHeaderType(bus, device, function);
    Device dev{bus, device, function, header_type, class_code};
    if(auto err = AddDevice(dev)){
      return err;
    }

    if(class_code.Match(0x06u, 0x04u)){
      auto bus_number = ReadBusNumbers(bus, device, function);
      uint8_t secondary_bus = (bus_number >> 8) & 0xffu;
      return ScanBus(secondary_bus);
    }

    return Error::kSuccess;
  }

  //指定デバイス以下にある有効なファンクションを検索
  Error ScanDevice(uint8_t bus, uint8_t device){
    if(auto err = ScanFunction(bus, device, 0)){
      return err;
    }
    if(IsSingleFunctionDevice(ReadHeaderType(bus, device, 0))){
      return Error::kSuccess;
    }

    for(uint8_t func=1; func<8; ++func){
      if(ReadVendorId(bus, device, func) == 0xffffu){
        continue;
      }
      if(auto err = ScanFunction(bus, device, func)){
        return err;
      }
    }
    return Error::kSuccess;
  }

  //指定バス以下にある有効なデバイスを探索
  Error ScanBus(uint8_t bus){
    for(uint8_t device=0; device<32; ++device){
      if(ReadVendorId(bus, device, 0) == 0xffffu){
        continue;
      }
      if(auto err = ScanDevice(bus, device)){
        return err;
      }
    }
    return Error::kSuccess;
  }
}


namespace pci {

  void WriteAddress(uint32_t address){
    IoOut32(kConfigAddress, address);
  }
  void WriteData(uint32_t value){
    IoOut32(kConfigData, value);
  }
  uint32_t ReadData(){
    return IoIn32(kConfigData);
  }
  uint16_t ReadVendorId(uint8_t bus, uint8_t device, uint8_t function){
    WriteAddress(MakeAddress(bus, device, function, 0x00));
    return ReadData() & 0xffffu;
  }
  uint16_t ReadDeviceId(uint8_t bus, uint8_t device, uint8_t function){
    WriteAddress(MakeAddress(bus, device, function, 0x00));
    return ReadData() >> 16;
  }
  uint8_t ReadHeaderType(uint8_t bus, uint8_t device, uint8_t function){
    WriteAddress(MakeAddress(bus, device, function, 0x0c));
    return (ReadData() >> 16) & 0xffu;
  }
  ClassCode ReadClassCode(uint8_t bus, uint8_t device, uint8_t function) {
    WriteAddress(MakeAddress(bus, device, function, 0x08));
    auto reg = ReadData();
    ClassCode cc;
    cc.base = (reg >> 24) & 0xffu;
    cc.sub = (reg >> 16) & 0xffu;
    cc.interface = (reg >> 8) & 0xffu;
    return cc;
  }
  uint32_t ReadBusNumbers(uint8_t bus, uint8_t device, uint8_t function) {
    WriteAddress(MakeAddress(bus, device, function, 0x18));
    return ReadData();
  }

  bool IsSingleFunctionDevice(uint8_t header_type) {
    return (header_type & 0x80u) == 0;
  }

  Error ScanAllBus(){
    num_device = 0;

    uint8_t header_type = ReadHeaderType(0,0,0);
    if(IsSingleFunctionDevice(header_type)){
      return ScanBus(0);
    }

    for(uint8_t func=1; func<8; ++func){
      if(ReadVendorId(0,0,func)==0xffffu){
        continue;
      }
      if(auto err = ScanBus(func)){
        return err;
      }
    }

    return Error::kSuccess;
  }
}
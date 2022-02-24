#pragma once

#include <cstdint>
#include <array>

#include "error.hpp"

namespace pci {
  const uint16_t kConfigAddress = 0x0cf8; //CONFIG_ADDRESS レジスタのIOポートアドレス
  const uint16_t kConfigData = 0x0cfc; //CONFIG_DATA レジスタのIOポートアドレス

  void WriteAddress(uint32_t address); //CONFIG_ADDRESS にアドレス指定
  void WriteData(uint32_t value); //CONFIG_DATA に値指定
  uint32_t ReadData(); //CONFIG_DATA から値読み出し
  uint16_t ReadVendorId(uint8_t bus, uint8_t device, uint8_t function); //ベンダID を取得
  uint16_t ReadDeviceId(uint8_t bus, uint8_t device, uint8_t function); //デバイスID を取得
  uint8_t ReadHeaderType(uint8_t bus, uint8_t device, uint8_t function); //ヘッダタイプ を取得
  uint32_t ReadClassCode(uint8_t bus, uint8_t device, uint8_t function); //クラスコード を取得
  uint32_t ReadBusNumbers(uint8_t bus, uint8_t device, uint8_t function); //バス番号 を取得

  /**
   * ClassCode
   *   - 31:24 : ベースクラス
   *   - 23:16 : サブクラス
   *   - 15:8  : インターフェース
   *   - 7:0   : リビジョン
   * BusNumbers
   *     23:16 : サブオーディネイトバス番号
   *   - 15:8  : セカンダリバス番号
   *   - 7:0   : リビジョン番号
   */
  
  bool IsSingleFunctionDevice(uint8_t header_type); //単一ファンクションであるか

  struct Device{
    uint8_t bus, device, function, header_type;
  };
  inline std::array<Device, 32> devices; //デバイス一覧
  inline int num_device; //デバイス数

  Error ScanAllBus();
}
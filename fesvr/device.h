#ifndef _DEVICE_H
#define _DEVICE_H

#include "term.h"
#include "packet.h"
#include <vector>
#include <queue>
#include <cstring>
#include <functional>

class htif_t;

class command_t
{
 public:
  typedef std::function<void(uint64_t)> callback_t;
  command_t(htif_t* htif, uint64_t tohost, callback_t cb)
    : _htif(htif), tohost(tohost), cb(cb) {}

  htif_t* htif() { return _htif; }
  uint8_t device() { return tohost >> 56; }
  uint8_t cmd() { return tohost >> 48; }
  uint64_t payload() { return tohost << 16 >> 16; }
  void respond(uint64_t resp) { cb((tohost >> 48 << 48) | (resp << 16 >> 16)); }

  static const size_t MAX_COMMANDS = 256;
  static const size_t MAX_DEVICES = 256;

 private:
  htif_t* _htif;
  uint64_t tohost;
  callback_t cb;
};

class device_t
{
 public:
  device_t();
  virtual ~device_t() {}
  virtual const char* identity() { return ""; }
  virtual void tick() {}

  void handle_command(command_t cmd);

 protected:
  typedef std::function<void(command_t)> command_func_t;
  void register_command(size_t, command_func_t, const char*);

 private:
  device_t& operator = (const device_t&); // disallow
  device_t(const device_t&); // disallow

  static const size_t IDENTITY_SIZE = 64;
  void handle_null_command(command_t cmd);
  void handle_identify(command_t cmd);

  std::vector<command_func_t> command_handlers;
  std::vector<std::string> command_names;
};

class bcd_t : public device_t
{
 public:
  bcd_t();
  const char* identity() { return "bcd"; }
  void tick();

 private:
  void handle_read(command_t cmd);
  void handle_write(command_t cmd);

  canonical_terminal_t term;
  std::queue<command_t> pending_reads;
};

class device_list_t
{
 public:
  device_list_t();
  void register_device(device_t* dev);
  void handle_command(command_t cmd);
  void tick();

 private:
  std::vector<device_t*> devices;
  device_t null_device;
  size_t num_devices;
};

#endif

// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include "leveldb/status.h"

#include <stdio.h>

#include "port/port.h"

namespace leveldb {

const char* Status::CopyState(const char* state) {
  uint32_t size;
  // 开了一个新的内存地址，前4个字节放state的前四个字节
  // 所以size的大小就是state前四个字节的值的大小，就是消息长度
  memcpy(&size, state, sizeof(size));
  // 新建了一个char数组，大小是size+5，
  // 因为消息本身长度是size，前五个字节放了消息长度和码值
  char* result = new char[size + 5];
  // 按字节拷贝
  memcpy(result, state, size + 5);
  return result;
}

Status::Status(Code code, const Slice& msg, const Slice& msg2) {
  assert(code != kOk);
  const uint32_t len1 = static_cast<uint32_t>(msg.size());
  const uint32_t len2 = static_cast<uint32_t>(msg2.size());
  const uint32_t size = len1 + (len2 ? (2 + len2) : 0);
  // [0-3] 消息长度，[4]码值，[5:]消息本身

  char* result = new char[size + 5];
  // size是一个uint32_t的数字，sizeof(unit_32_t)是4B
  // 前四个字节存放消息长度
  memcpy(result, &size, sizeof(size));
  // 存放码值
  result[4] = static_cast<char>(code);
  // 存消息1本身
  memcpy(result + 5, msg.data(), len1);
  // 如果有消息2，接一个冒号再存
  if (len2) {
    result[5 + len1] = ':';
    result[6 + len1] = ' ';
    memcpy(result + 7 + len1, msg2.data(), len2);
  }
  // state_ 重新指向result
  state_ = result;
}

std::string Status::ToString() const {
  if (state_ == nullptr) {
    return "OK";
  } else {
    char tmp[30];
    // type是一个指向const char的指针
    // 可以让type指向新的字符串，但是不能通过这个指针改变这个字符串
    const char* type;
    switch (code()) {
      case kOk:
        type = "OK";
        break;
      case kNotFound:
        type = "NotFound: ";
        break;
      case kCorruption:
        type = "Corruption: ";
        break;
      case kNotSupported:
        type = "Not implemented: ";
        break;
      case kInvalidArgument:
        type = "Invalid argument: ";
        break;
      case kIOError:
        type = "IO error: ";
        break;
      default:
        snprintf(tmp, sizeof(tmp),
                 "Unknown code(%d): ", static_cast<int>(code()));
        type = tmp;
        break;
    }
    std::string result(type);
    uint32_t length;
    memcpy(&length, state_, sizeof(length));
    // 在字符串尾部添加state_的子串，从5开始，长length个
    result.append(state_ + 5, length);
    return result;
  }
}

}  // namespace leveldb

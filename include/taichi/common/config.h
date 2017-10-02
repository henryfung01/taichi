/*******************************************************************************
    Taichi - Physically based Computer Graphics Library

    Copyright (c) 2016 Yuanming Hu <yuanmhu@gmail.com>

    All rights reserved. Use of this source code is governed by
    the MIT license as written in the LICENSE file.
*******************************************************************************/

#pragma once

#include <map>
#include <string>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <typeinfo>

#include <taichi/common/string_utils.h>
#include <taichi/common/util.h>
#include <taichi/common/asset_manager.h>
#include <taichi/math/math.h>

TC_NAMESPACE_BEGIN

// Declare and then load
//#define TC_PULL_CONFIG(name, default_val) \
  //decltype(default_val) name = config.get(#name, default_val)
// Load to `this`
//#define TC_LOAD_CONFIG(name, default_val) \
//  this->name = config.get(#name, default_val)

class Config {
 private:
  std::map<std::string, std::string> data;
  std::vector<std::string> file_names;

 public:
  Config() {}

  void clear() {
    data.clear();
    file_names.clear();
  }

  template <typename T>
  T get(std::string key) const;

  void print_all() const {
    std::cout << "Configures: " << std::endl;
    for (auto key = data.begin(); key != data.end(); key++) {
      std::cout << " * " << key->first << " = " << key->second << std::endl;
    }
  }

  float get_float(std::string key) const {
    return (float)std::atof(get_string(key).c_str());
  }

  double get_double(std::string key) const {
    return (double)std::atof(get_string(key).c_str());
  }

  real get_real(std::string key) const {
    return (real)std::atof(get_string(key).c_str());
  }

  int get_int(std::string key) const {
    return std::atoi(get_string(key).c_str());
  }

  int64 get_int64(std::string key) const {
    return std::atoll(get_string(key).c_str());
  }

  unsigned get_unsigned(std::string key) const {
    return unsigned(std::atoll(get_string(key).c_str()));
  }

  template <typename V, int N = V::N, typename T=typename V::ScalarType>
  VectorND<N, T> get(std::string key) const {
    std::string str = this->get_string(key);
    std::string temp = "(";
    for (int i = 0; i < N; i++) {
      std::string placeholder;
      if (std::is_same<T, float32>()) {
        placeholder = "%f";
      } else if (std::is_same<T, float64>()) {
        placeholder = "%lf";
      } else if (std::is_same<T, int32>()) {
        placeholder = "%d";
      } else if (std::is_same<T, uint32>()) {
        placeholder = "%u";
      } else if (std::is_same<T, int64>()) {
#ifdef WIN32
        placeholder = "%I64d";
#else
        placeholder = "%lld";
#endif
      } else if (std::is_same<T, uint64>()) {
#ifdef WIN32
        placeholder = "%I64u";
#else
        placeholder = "%llu";
#endif
      } else {
        assert(false);
      }
      temp += placeholder;
      if (i != N - 1) {
        temp += ",";
      }
    }
    temp += ")";
    VectorND<N, T> ret;
    if (N == 1) {
      sscanf(str.c_str(), temp.c_str(), &ret[0]);
    } else if (N == 2) {
      sscanf(str.c_str(), temp.c_str(), &ret[0], &ret[1]);
    }else if (N == 3) {
      sscanf(str.c_str(), temp.c_str(), &ret[0], &ret[1], &ret[2]);
    }else if (N == 4) {
      sscanf(str.c_str(), temp.c_str(), &ret[0], &ret[1], &ret[2], &ret[3]);
    }
    return ret;
  }

  template  <typename T>
  T get(std::string key, const T &default_val) const {
    if (data.find(key) == data.end()) {
      return default_val;
    } else
      return get<T>(key);
  }

  bool has_key(std::string key) const { return data.find(key) != data.end(); }

  std::vector<std::string> get_string_arr(std::string key) const {
    std::string str = get_string(key);
    std::vector<std::string> strs = split_string(str, ",");
    for (auto &s : strs) {
      s = trim_string(s);
    }
    return strs;
  }

  template <typename T>
  T *get_ptr(std::string key) const {
    std::string val = get_string(key);
    std::stringstream ss(val);
    std::string t;
    int64 ptr_ll;
    std::getline(ss, t, '\t');
    ss >> ptr_ll;
    assert_info(t == typeid(T).name(),
                "Pointer type mismatch: " + t + " and " + typeid(T).name());
    return reinterpret_cast<T *>(ptr_ll);
  }

  bool get_bool(std::string key) const {
    std::string s = get_string(key);
    static std::map<std::string, bool> dict{
        {"true", true},   {"True", true},   {"t", true},  {"1", true},
        {"false", false}, {"False", false}, {"f", false}, {"0", false},
    };
    assert_info(dict.find(s) != dict.end(), "Unkown identifer for bool: " + s);
    return dict[s];
  }

  template <typename T>
  std::shared_ptr<T> get_asset(std::string key) const {
    int id = get_int(key);
    return AssetManager::get_asset<T>(id);
  }

  template <typename T>
  Config &set(std::string name, T val) {
    std::stringstream ss;
    ss << val;
    data[name] = ss.str();
    return *this;
  }

  Config &set(std::string name, const char *val) {
    std::stringstream ss;
    ss << val;
    data[name] = ss.str();
    return *this;
  }

  Config &set(std::string name, const Vector2 &val) {
    std::stringstream ss;
    ss << "(" << val.x << "," << val.y << ")";
    data[name] = ss.str();
    return *this;
  }

  Config &set(std::string name, const Vector3 &val) {
    std::stringstream ss;
    ss << "(" << val.x << "," << val.y << "," << val.z << ")";
    data[name] = ss.str();
    return *this;
  }

  Config &set(std::string name, const Vector4 &val) {
    std::stringstream ss;
    ss << "(" << val.x << "," << val.y << "," << val.z << "," << val.w << ")";
    data[name] = ss.str();
    return *this;
  }

  Config &set(std::string name, const Vector2i &val) {
    std::stringstream ss;
    ss << "(" << val.x << "," << val.y << ")";
    data[name] = ss.str();
    return *this;
  }

  Config &set(std::string name, const Vector3i &val) {
    std::stringstream ss;
    ss << "(" << val.x << "," << val.y << "," << val.z << ")";
    data[name] = ss.str();
    return *this;
  }

  Config &set(std::string name, const Vector4i &val) {
    std::stringstream ss;
    ss << "(" << val.x << "," << val.y << "," << val.z << "," << val.w << ")";
    data[name] = ss.str();
    return *this;
  }

  template <typename T>
  static std::string get_ptr_string(T *ptr) {
    std::stringstream ss;
    ss << typeid(T).name() << "\t" << reinterpret_cast<uint64>(ptr);
    return ss.str();
  }

  template <typename T>
  Config &set(std::string name, T *const ptr) {
    data[name] = get_ptr_string(ptr);
    return *this;
  }

  std::string get_all_file_names() const {
    std::string ret = "";
    for (auto f : file_names)
      ret += f + " ";
    return ret;
  }

  std::string get_string(std::string key) const {
    if (data.find(key) == data.end()) {
      assert_info(false,
                  "No key named '" + key +
                      "' found! [Config files: " + get_all_file_names() + "]");
    }
    return data.find(key)->second;
  }
};

TC_NAMESPACE_END

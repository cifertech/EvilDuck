#include "duck_vars.h"

static const size_t MAX_VARS = 16;
static String varNames[MAX_VARS];
static String varValues[MAX_VARS];
static size_t varCount = 0;

namespace duck_vars {

void clear() {
  varCount = 0;
}

bool set(const String &name, const String &value) {
  String key = name;
  key.trim();
  if (!key.length() || key[0] != '$') return false;
  for (size_t i = 0; i < varCount; ++i) {
    if (varNames[i] == key) {
      varValues[i] = value;
      return true;
    }
  }
  if (varCount >= MAX_VARS) return false;
  varNames[varCount] = key;
  varValues[varCount] = value;
  ++varCount;
  return true;
}

bool get(const String &name, String &out) {
  String key = name;
  key.trim();
  for (size_t i = 0; i < varCount; ++i) {
    if (varNames[i] == key) {
      out = varValues[i];
      return true;
    }
  }
  return false;
}

String expand(const String &input) {
  String out;
  out.reserve(input.length());
  for (size_t i = 0; i < input.length(); ++i) {
    if (input[i] == '$') {
      size_t j = i + 1;
      while (j < input.length() &&
             (isalnum(static_cast<unsigned char>(input[j])) || input[j] == '_')) ++j;
      if (j > i + 1) {
        String key = input.substring(i, j);
        String val;
        if (get(key, val)) {
          out += val;
          i = j - 1;
          continue;
        }
      }
    }
    out += input[i];
  }
  return out;
}

bool truthy(const String &name) {
  String v;
  if (!get(name, v)) return false;
  v.trim();
  v.toLowerCase();
  return v == "1" || v == "true" || v == "yes" || (v.length() > 0 && v != "0" && v != "false");
}

}

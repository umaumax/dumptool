#include <fstream>
#include <iostream>
#include <iterator>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace dumptool {
class StatusCode {
 public:
  enum Value : uint8_t {
    kOK = 0,
    kInvalidArgs,
    kOutOfRangeAccess,
    kNotFoundFile,
  };
  const std::string ToString() const {
    switch (value) {
      case kOK:
        return "kOK";
      case kInvalidArgs:
        return "kInvalidArgs";
      case kOutOfRangeAccess:
        return "kOutOfRangeAccess";
      case kNotFoundFile:
        return "kNotFoundFile";
      default:
        break;
    }
    assert(false);
    return "";
  }

  StatusCode() = default;
  constexpr StatusCode(Value v) : value(v) {}
  operator int() const { return static_cast<int>(value); }

  bool IsOK() const { return value == kOK; }
  const int ToInt() const { return static_cast<int>(value); }

  bool operator<<(StatusCode a) const { return value == a.value; }
  bool operator==(StatusCode a) const { return value == a.value; }
  bool operator!=(StatusCode a) const { return value != a.value; }

 private:
  friend std::ostream& operator<<(std::ostream& os,
                                  const StatusCode& status_code);
  Value value;
};

std::ostream& operator<<(std::ostream& os, const StatusCode& status_code) {
  return os << status_code.ToString() << "(" << status_code.ToInt() << ")";
}

class StatusValue {
 public:
  StatusValue() : code(StatusCode::kOK), message("") {}
  StatusValue(const StatusCode code, const std::string& message)
      : code(code), message(message) {}

  StatusCode code;
  std::string message;

  bool IsOK() { return code.IsOK(); }
  int ExitCode() { return code; }

  std::string ToString() {
    std::stringstream ss;
    ss << code << ":" << message;
    return ss.str();
  }
};

class Status {
 public:
  Status() {}
  Status(const StatusCode code, const std::string& message) {
    statuses.emplace_back(StatusValue(code, message));
  }

  std::vector<StatusValue> statuses;

  bool IsOK() { return statuses.size() == 0 || statuses[0].IsOK(); }
  int ExitCode() {
    if (statuses.size() == 0) {
      return StatusCode::kOK;
    }
    return statuses[0].ExitCode();
  }
  Status Wrap(Status status);

  std::string ToString() {
    std::stringstream ss;
    for (auto&& status : statuses) {
      ss << status.ToString() << std::endl;
    }
    return ss.str();
  }
};

Status Status::Wrap(Status status) {
  for (auto&& status_value : status.statuses) {
    statuses.emplace_back(status_value);
  }
  return *this;
}  // namespace dumptool

#define DUMPTOOL_STREAM_STR(stream)           \
  [&]() {                                     \
    std::stringstream DUMPTOOL_STREAM_STR_ss; \
    DUMPTOOL_STREAM_STR_ss << stream;         \
    return DUMPTOOL_STREAM_STR_ss.str();      \
  }()

// #define DUMPTOOL_DECL_GEN_STATUS_FUNC(status)              \
  // Status Gen##status##Status(const std::string& message) { \
  // return Status(StatusCode::k##status, message);         \
  // }
//
// DUMPTOOL_DECL_GEN_STATUS_FUNC(OutOfRangeAccess);
// DUMPTOOL_DECL_GEN_STATUS_FUNC(InvalidArgs);
// DUMPTOOL_DECL_GEN_STATUS_FUNC(OK);

// --------------------------------

std::vector<std::string> Split(std::string text, std::string delims) {
  std::vector<std::string> vec;
  std::string::size_type spos = 0, epos;
  if (text == "") return vec;
  while ((epos = text.find_first_of(delims, spos)) != std::string::npos) {
    vec.emplace_back(text.substr(spos, epos - spos));
    spos = epos + 1;
  }
  vec.emplace_back(text.substr(spos));
  return vec;
}

Status Print(const std::string& format, const std::vector<uint8_t>& raw) {
  std::vector<std::string> field_formats = Split(format, ",");
  uint8_t* p                             = const_cast<uint8_t*>(raw.data());
  uint8_t* start                         = p;
  std::size_t size                       = raw.size();
  for (auto&& field_format : field_formats) {
    std::vector<std::string> keys = Split(field_format, ":");
    std::string name;
    std::string type;
    int array_size = 1;
    if (keys.size() == 1) {
      type = keys[0];
    } else if (keys.size() == 2) {
      name = keys[0];
      type = keys[1];
    } else {
      return dumptool::Status(
          dumptool::StatusCode::kInvalidArgs,
          DUMPTOOL_STREAM_STR("invalid number of field keys:" << field_format));
    }

    std::regex array_regex("^(.*)\\[(.+)\\]$");
    std::regex int_regex("^(0|[1-9][0-9]*)$");
    std::smatch m;
    if (std::regex_match(type, m, array_regex)) {
      // NOTE: assign to num_str before target string
      std::string num_str = m[2].str();
      type                = m[1].str();
      if (std::regex_match(num_str, int_regex)) {
        array_size = std::stoi(num_str);
      } else {
        return dumptool::Status(
            dumptool::StatusCode::kInvalidArgs,
            DUMPTOOL_STREAM_STR("invalid number format:" << num_str));
      }
    }

    if (name != "") {
      std::cout << name << ":";
    }
    if (array_size == 1) {
      std::cout << type << ":";
    } else {
      std::cout << type << "[" << array_size << "]"
                << ":";
    }
    for (int i = 0; i < array_size; i++) {
      // NOTE: 現状では，表示した後にサイズ比較を行っている
      // 本来は表示前に比較するべき
      if (p - start >= size) {
        std::cout << std::endl;
        return dumptool::Status(
            dumptool::StatusCode::kOutOfRangeAccess,
            DUMPTOOL_STREAM_STR("raw data size is " << size));
      }
      if (type == "skip") {
        p += 1;
        continue;
      }
      if (type == "offset") {
        p = start + array_size;
        break;
      }
      if (i > 0) {
        std::cout << ", ";
      }
      if (type == "int") {
        std::cout << *(int*)p;
        p += sizeof(int);
      } else if (type == "double") {
        std::cout << *(double*)p;
        p += sizeof(double);
      } else if (type == "float") {
        std::cout << *(float*)p;
        p += sizeof(float);
      } else if (type == "char") {
        std::cout << *(char*)p;
        p += sizeof(char);
      } else if (type == "int8_t") {
        std::cout << (int32_t) * (int8_t*)p;
        p += sizeof(int8_t);
      } else if (type == "uint8_t") {
        std::cout << (uint32_t) * (uint8_t*)p;
        p += sizeof(uint8_t);
      } else if (type == "int16_t") {
        std::cout << *(int16_t*)p;
        p += sizeof(int16_t);
      } else if (type == "uint16_t") {
        std::cout << *(uint16_t*)p;
        p += sizeof(uint16_t);
      } else if (type == "int32_t") {
        std::cout << *(int32_t*)p;
        p += sizeof(int32_t);
      } else if (type == "uint32_t") {
        std::cout << *(uint32_t*)p;
        p += sizeof(uint32_t);
      } else if (type == "int64_t") {
        std::cout << *(int64_t*)p;
        p += sizeof(int64_t);
      } else if (type == "uint64_t") {
        std::cout << *(uint64_t*)p;
        p += sizeof(uint64_t);
      } else if (type == "string") {
        std::size_t str_size;
        // NOTE: search null
        for (str_size = 0; p + str_size < start + size && p[str_size] != 0;
             str_size++) {
        }
        std::cout << (char*)p;
        // NOTE: +1 means null
        p += str_size + 1;
      } else {
        return dumptool::Status(dumptool::StatusCode::kInvalidArgs,
                                DUMPTOOL_STREAM_STR("unknown type:" << type));
      }
    }
    std::cout << std::endl;
  }
  return dumptool::Status(dumptool::StatusCode::kOK,
                          DUMPTOOL_STREAM_STR("success"));
}

Status Print(const std::string& format, const std::string& filepath) {
  std::ifstream file(filepath, std::ios::binary);
  if (file.fail()) {
    return dumptool::Status(dumptool::StatusCode::kNotFoundFile,
                            DUMPTOOL_STREAM_STR("not found file:" << filepath));
  }
  file.unsetf(std::ios::skipws);
  std::streampos file_size;
  file.seekg(0, std::ios::end);
  file_size = file.tellg();
  file.seekg(0, std::ios::beg);
  std::vector<uint8_t> vec;
  vec.reserve(file_size);
  vec.insert(vec.begin(), std::istream_iterator<uint8_t>(file),
             std::istream_iterator<uint8_t>());
  return Print(format, vec);
}
}  // namespace dumptool

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << argv[0] << " <format> [filepath]" << std::endl;
    return 1;
  }
  std::string format = std::string(argv[1]);
  std::string filepath;
  dumptool::Status status;
  if (argc >= 3) {
    filepath = std::string(argv[2]);
    status   = dumptool::Print(format, filepath);
  } else {
    // NOTE: example
    // NOTE: little endian
    std::vector<uint8_t> raw = {0x01, 0x00, 0x00, 0x00};
    // *(int*)(raw.data())      = 0x01;
    status = dumptool::Print(format, raw);
  }
  if (!status.IsOK()) {
    std::cerr << status.ToString() << std::endl;
    return status.ExitCode();
  }
  return 0;
}

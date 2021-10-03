#pragma once

#include <nlohmann/json.hpp>
#include <optional>

namespace nlohmann {
  template<typename T>
  struct adl_serializer<std::optional<T>> {
    static void to_json(json &j, const std::optional<T> &opt) {
      if (!opt.has_value()) {
        j = nullptr;
      } else {
        j = *opt;
      }
    }

    static void from_json(const json &j, std::optional<T> &opt) {
      if (j.is_null()) {
        opt = {};
      } else {
        opt = j.get<T>();
      }
    }
  };
}
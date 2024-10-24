#pragma once

#include <functional>
#include <map>
#include <optional>
#include <stdexcept>
#include <variant>
#include <vector>

#include "types.h"

namespace gameconsole {

// Forward declaration
class Collection;
using CollectionPtr = std::shared_ptr<Collection>;

class Collection {
public:
  // 使用 std::variant 替代 Godot 的 Variant

protected:
  std::map<Variant, Variant> _collection;
  int _iterationCurrent = -1;
  int _maxId = 0;

public:
  Collection() = default;

  // 初始化集合
  void initial(const Variant &p_collection) {
    _collection = to_dict(p_collection);
  }

  // 设置键值对
  void set_value(const Variant &key, const Variant &value) {
    _collection[key] = value;
  }

  // 添加值(使用size作为key)
  void add(const Variant &value) {
    _maxId = std::max(_maxId, static_cast<int>(_collection.size()));
    set_value(_maxId++, value);
  }

  // 移除指定key的元素
  void remove(const Variant &key) { _collection.erase(key); }

  // 移除指定值的元素
  bool remove_element(const Variant &element) {
    for (auto it = _collection.begin(); it != _collection.end(); ++it) {
      if (it->second == element) {
        _collection.erase(it);
        return true;
      }
    }
    return false;
  }

  // 通过索引移除元素
  void remove_by_index(int index) {
    if (index >= 0 && index < static_cast<int>(_collection.size())) {
      auto it = std::next(_collection.begin(), index);
      _collection.erase(it);
    }
  }

  // 检查是否包含指定的key
  bool contains_key(const Variant &key) const {
    return _collection.find(key) != _collection.end();
  }

  // 检查是否包含指定的值
  bool contains(const Variant &element) const {
    for (const auto &pair : _collection) {
      if (pair.second == element) {
        return true;
      }
    }
    return false;
  }

  // 获取元素的索引
  std::optional<Variant> index_of(const Variant &element) const {
    for (const auto &pair : _collection) {
      if (pair.second == element) {
        return pair.first;
      }
    }
    return std::nullopt;
  }

  // 获取指定key的值
  std::optional<Variant> get_value(const Variant &key) const {
    auto it = _collection.find(key);
    if (it != _collection.end()) {
      return it->second;
    }
    return std::nullopt;
  }

  // 通过索引获取值
  std::optional<Variant> get_by_index(int index) const {
    if (index >= 0 && index < static_cast<int>(_collection.size())) {
      auto it = std::next(_collection.begin(), index);
      return it->second;
    }
    return std::nullopt;
  }

  // 获取所有键
  std::vector<Variant> get_keys() const {
    std::vector<Variant> keys;
    for (const auto &pair : _collection) {
      keys.push_back(pair.first);
    }
    return keys;
  }

  // 获取所有值
  std::vector<Variant> get_values() const {
    std::vector<Variant> values;
    for (const auto &pair : _collection) {
      values.push_back(pair.second);
    }
    return values;
  }

  // 检查集合是否为空
  bool is_empty() const { return _collection.empty(); }

  // 清空集合
  void clear() { _collection.clear(); }

  // 获取第一个元素
  std::optional<Variant> first() {
    if (!is_empty()) {
      _iterationCurrent = 0;
      return get_by_index(_iterationCurrent);
    }
    return std::nullopt;
  }

  // 获取最后一个元素
  std::optional<Variant> last() {
    if (!is_empty()) {
      _iterationCurrent = static_cast<int>(_collection.size()) - 1;
      return get_by_index(_iterationCurrent);
    }
    return std::nullopt;
  }

  // 设置当前迭代位置
  void seek(int id) { _iterationCurrent = id; }

  // 检查是否还有下一个元素
  bool has_next() const {
    return _iterationCurrent < static_cast<int>(_collection.size()) - 1;
  }

  // 获取下一个元素
  std::optional<Variant> next() {
    if (!is_empty() && has_next()) {
      _iterationCurrent++;
      return get_by_index(_iterationCurrent);
    }
    return std::nullopt;
  }

  // 获取当前元素
  std::optional<Variant> current() {
    if (!is_empty() && _iterationCurrent >= 0) {
      return get_by_index(_iterationCurrent);
    }
    return std::nullopt;
  }

  // 获取前一个元素
  std::optional<Variant> previous() {
    if (!is_empty() && _iterationCurrent > 0) {
      _iterationCurrent--;
      return get_by_index(_iterationCurrent);
    }
    return std::nullopt;
  }

  // 获取集合大小
  size_t size() const { return _collection.size(); }

  CollectionPtr fill(const Variant &value = std::monostate{},
                     int startIndex = 0,
                     const Variant &length = std::monostate{}) {
    int fill_length;

    if (std::holds_alternative<std::monostate>(length)) {
      fill_length = static_cast<int>(_collection.size()) - startIndex;
    } else {
      fill_length = std::get<int>(length);
    }

    for (int i = startIndex; i < startIndex + fill_length &&
                             i < static_cast<int>(_collection.size());
         i++) {
      _collection[i] = value;
    }

    return std::make_shared<Collection>(*this);
  }

  // 转换为字典的辅助函数
  std::map<Variant, Variant> to_dict(const Variant &value) const {
    std::map<Variant, Variant> d;

    if (auto map_ptr = std::get_if<std::shared_ptr<VariantMap>>(&value)) {
      return (*map_ptr)->values;
    } else if (!std::holds_alternative<std::monostate>(value)) {
      if (auto arr_ptr = std::get_if<std::shared_ptr<VariantArray>>(&value)) {
        const auto &arr = (*arr_ptr)->values;
        for (size_t i = 0; i < arr.size(); i++) {
          d[static_cast<int>(i)] = arr[i];
        }
      } else {
        d[0] = value;
      }
    }
    return d;
  }

  // filter 函数实现
  std::shared_ptr<Collection>
  filter(std::function<bool(const Variant &, const Variant &, int,
                            const Collection &)>
             callback = nullptr) {
    auto new_collection = std::make_shared<Collection>();
    new_collection->_collection = _collection;

    int i = 0;
    if (callback) {
      while (i < static_cast<int>(new_collection->_collection.size())) {
        auto keys = new_collection->get_keys();
        Variant key = keys[i];
        Variant value =
            new_collection->get_value(key).value_or(std::monostate{});

        bool result = callback(key, value, i, *new_collection);

        if (!result) {
          new_collection->remove_by_index(i);
        } else {
          i++;
        }
      }
    } else {
      while (i < static_cast<int>(new_collection->_collection.size())) {
        auto value_opt = new_collection->get_by_index(i);
        if (!value_opt.has_value()) {
          new_collection->remove_by_index(i);
          continue;
        }

        const Variant &value = value_opt.value();
        bool should_remove = false;

        if (std::holds_alternative<std::monostate>(value)) {
          should_remove = true;
        } else if (std::holds_alternative<std::string>(value)) {
          should_remove = std::get<std::string>(value).empty();
        } else if (auto arr_ptr =
                       std::get_if<std::shared_ptr<VariantArray>>(&value)) {
          should_remove = (*arr_ptr)->values.empty();
        } else if (auto map_ptr =
                       std::get_if<std::shared_ptr<VariantMap>>(&value)) {
          should_remove = (*map_ptr)->values.empty();
        }

        if (should_remove) {
          new_collection->remove_by_index(i);
        } else {
          i++;
        }
      }
    }

    return new_collection;
  }

  // 辅助函数：创建数组
  static Variant make_array(const std::vector<Variant> &values) {
    auto arr = std::make_shared<VariantArray>();
    arr->values = values;
    return arr;
  }

  // 辅助函数：创建字典
  static Variant make_map(const std::map<Variant, Variant> &values) {
    auto map = std::make_shared<VariantMap>();
    map->values = values;
    return map;
  }
};

} // namespace gameconsole
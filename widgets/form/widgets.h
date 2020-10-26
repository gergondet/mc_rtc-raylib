#pragma once

#include "../Widget.h"

#include <memory>

namespace form
{

struct Widget
{
  Widget(const ::Widget & parent, const std::string & name) : parent_(parent), name(name) {}

  virtual ~Widget() = default;

  virtual bool ready() = 0;

  virtual void draw() = 0;

  virtual std::string value() = 0;

  virtual void collect(mc_rtc::Configuration & out) = 0;

  template<typename T = const char *>
  inline std::string label(std::string_view label, T && suffix = "")
  {
    return fmt::format("{}##{}{}{}{}", label, parent_.id.category, parent_.id.name, name, suffix);
  }

  std::string name;
  bool required;

protected:
  const ::Widget & parent_;
};

template<typename DataT>
struct SimpleInput : public Widget
{
  SimpleInput(const ::Widget & parent, const std::string & name, const DataT & value)
  : Widget(parent, name), value_(value)
  {
  }

  ~SimpleInput() override = default;

  bool ready() override
  {
    if constexpr(std::is_same_v<DataT, std::string>)
    {
      return value_.size() != 0;
    }
    else
    {
      return true;
    }
  }

  void collect(mc_rtc::Configuration & out) override
  {
    out.add(name, value_);
  }

  std::string value() override
  {
    if constexpr(std::is_same_v<DataT, std::string>)
    {
      return value_;
    }
    else
    {
      return fmt::format("{}", value_);
    }
  }

protected:
  DataT value_;
};

struct Checkbox : public SimpleInput<bool>
{
  using SimpleInput::SimpleInput;

  inline void draw() override
  {
    ImGui::Checkbox(label(name).c_str(), &value_);
  }
};

struct IntegerInput : public SimpleInput<int>
{
  using SimpleInput::SimpleInput;

  inline void draw() override
  {
    ImGui::InputInt(label(name).c_str(), &value_, 0, 0);
  }
};

struct NumberInput : public SimpleInput<double>
{
  using SimpleInput::SimpleInput;

  inline void draw() override
  {
    ImGui::InputDouble(label(name).c_str(), &value_);
  }
};

struct StringInput : public SimpleInput<std::string>
{
  using SimpleInput::SimpleInput;

  inline void draw() override
  {
    if(buffer_.size() < std::max<size_t>(value_.size() + 1, 256))
    {
      buffer_.resize(std::max<size_t>(value_.size() + 1, 256));
      std::memcpy(buffer_.data(), value_.data(), value_.size());
      buffer_[value_.size()] = '0';
    }
    if(ImGui::InputText(label(name).c_str(), buffer_.data(), buffer_.size()))
    {
      value_ = {buffer_.data(), strnlen(buffer_.data(), buffer_.size())};
    }
  }

private:
  std::vector<char> buffer_;
};

struct ArrayInput : public SimpleInput<Eigen::VectorXd>
{
  ArrayInput(const ::Widget & parent, const std::string & name, const Eigen::VectorXd & default_, bool fixed_size);

  void draw() override;

private:
  bool fixed_;
};

struct ComboInput : public SimpleInput<std::string>
{
  ComboInput(const ::Widget & parent,
             const std::string & name,
             const std::vector<std::string> & values,
             bool send_index);

  void draw() override;

  inline void collect(mc_rtc::Configuration & out) override
  {
    if(send_index_)
    {
      out.add(name, idx_);
    }
    else
    {
      out.add(name, value_);
    }
  }

protected:
  std::vector<std::string> values_;
  size_t idx_;
  bool send_index_;

  void draw(const char * label);
};

struct DataComboInput : public ComboInput
{
  DataComboInput(const ::Widget & parent,
                 const std::string & name,
                 const std::vector<std::string> & ref,
                 bool send_index);

  void draw() override;

protected:
  std::vector<std::string> ref_;
};

using WidgetPtr = std::unique_ptr<Widget>;

} // namespace form

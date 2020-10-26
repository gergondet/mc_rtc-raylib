#include "widgets.h"

#include "../Form.h"

namespace form
{

ArrayInput::ArrayInput(const ::Widget & parent,
                       const std::string & name,
                       const Eigen::VectorXd & default_,
                       bool fixed_size)
: SimpleInput::SimpleInput(parent, name, default_), fixed_(fixed_size)
{
}

void ArrayInput::draw()
{
  ImGui::Columns(2);
  for(size_t i = 0; i < value_.size(); ++i)
  {
    ImGui::InputDouble(label(fmt::format("{}", i)).c_str(), &value_(i));
    if(!fixed_)
    {
      ImGui::SameLine();
      if(ImGui::Button(label("-", i).c_str()))
      {
        Eigen::VectorXd nValue = Eigen::VectorXd::Zero(value_.size() - 1);
        if(i != 0)
        {
          nValue.head(i) = value_.head(i);
        }
        if(nValue.size() - i)
        {
          nValue.tail(nValue.size() - i) = value_.tail(value_.size() - 1 - i);
        }
        value_ = nValue;
      }
    }
  }
  if(!fixed_)
  {
    if(ImGui::Button(label("+").c_str()))
    {
      Eigen::VectorXd nValue = Eigen::VectorXd::Zero(value_.size() + 1);
      nValue.head(value_.size()) = value_;
      value_ = nValue;
    }
  }
  ImGui::NextColumn();
  ImGui::Text("%s", name.c_str());
  ImGui::Columns(1);
}

ComboInput::ComboInput(const ::Widget & parent,
                       const std::string & name,
                       const std::vector<std::string> & values,
                       bool send_index)
: SimpleInput(parent, name, ""), values_(values), idx_(values_.size()), send_index_(send_index)
{
}

void ComboInput::draw()
{
  const char * label = idx_ < values_.size() ? values_[idx_].c_str() : "";
  draw(label);
}

void ComboInput::draw(const char * label_)
{
  if(ImGui::BeginCombo(label(name).c_str(), label_))
  {
    for(size_t i = 0; i < values_.size(); ++i)
    {
      if(ImGui::Selectable(values_[i].c_str(), idx_ == i))
      {
        idx_ = i;
        value_ = values_[i];
      }
      if(idx_ == i)
      {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
}

DataComboInput::DataComboInput(const ::Widget & parent,
                               const std::string & name,
                               const std::vector<std::string> & ref,
                               bool send_index)
: ComboInput(parent, name, {}, send_index), ref_(ref)
{
}

void DataComboInput::draw()
{
  const auto & form = static_cast<const Form &>(parent_);
  auto data = form.client.data();
  std::string label;
  auto resolve = [&]() -> std::vector<std::string> {
    for(size_t i = 0; i < ref_.size(); ++i)
    {
      std::string ref = ref_[i];
      if(ref.size() && ref[0] == '$')
      {
        ref = form.value(ref.substr(1));
      }
      if(!data.has(ref))
      {
        if(ref_[i].size() && ref_[i][0] == '$')
        {
          label = fmt::format("Fill {} first", ref_[i].substr(1));
        }
        else
        {
          std::string full_ref = "";
          for(size_t j = 0; j <= i; ++j)
          {
            full_ref += ref_[j];
            if(j != i)
            {
              full_ref += "/";
            }
          }
          label = fmt::format("No {} entry in the data provided by the server", full_ref);
        }
        return {};
      }
      data = data(ref);
    }
    return data;
  };
  values_ = resolve();
  if(idx_ >= values_.size() || values_[idx_] != value_)
  {
    idx_ = values_.size();
    value_ = "";
  }
  const char * label_ = label.size() ? label.c_str() : value_.c_str();
  ComboInput::draw(label_);
}

} // namespace form

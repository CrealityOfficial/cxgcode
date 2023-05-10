#pragma once

#ifndef CXCLOUD_DEFINE_HPP_
#define CXCLOUD_DEFINE_HPP_

#include <QString>

namespace cxcloud {

struct UserBaseInfo {
  QString token{};
  QString user_id{};
};

struct UserDetailInfo {
  QString avatar{};
  QString nick_name{};
  double max_storage_space{0.0};
  double used_storage_space{0.0};
};

}  // namespace cxcloud

#endif  // CXCLOUD_DEFINE_HPP_
